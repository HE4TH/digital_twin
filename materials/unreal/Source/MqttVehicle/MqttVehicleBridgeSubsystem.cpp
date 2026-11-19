// MqttVehicleBridgeSubsystem.cpp  (v3 - UE 5.7 실제 MQTT API 기준)

#include "MqttVehicleBridgeSubsystem.h"

#include "Async/Async.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// ---- 엔진 MQTT 플러그인 ----------------------------------------------------
//  UE 5.7 경로: Engine\Plugins\Protocols\MQTT\Source\MQTTCore\Public
//  모듈 이름  : MQTTCore  (Build.cs 의존성에 들어 있어야 함)
#include "IMQTTCoreModule.h"
#include "IMQTTClient.h"
#include "MQTTClientMessage.h"
#include "MQTTShared.h"
// ---------------------------------------------------------------------------

DEFINE_LOG_CATEGORY_STATIC(LogMqttVehicle, Log, All);

// ----------------------------------------------------------------------------
// 생명주기
// ----------------------------------------------------------------------------

bool UMqttVehicleBridgeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Editor / EditorPreview 월드에서는 아예 생성하지 않는다.
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMqttVehicleBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		UE_LOG(LogMqttVehicle, Verbose, TEXT("MQTT vehicle bridge skipped: not a game world."));
		return;
	}

	ConnectAndSubscribe();
}

void UMqttVehicleBridgeSubsystem::Deinitialize()
{
	if (Client.IsValid())
	{
		if (MessageHandle.IsValid())
		{
			Client->OnMessage().Remove(MessageHandle);
			MessageHandle.Reset();
		}
		Client->Disconnect();
		Client.Reset();
	}

	bConnected = false;
	UE_LOG(LogMqttVehicle, Log, TEXT("MQTT vehicle bridge shut down."));

	Super::Deinitialize();
}

TStatId UMqttVehicleBridgeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMqttVehicleBridgeSubsystem, STATGROUP_Tickables);
}

bool UMqttVehicleBridgeSubsystem::IsTickable() const
{
	return bConnected && !IsTemplate();
}

// ----------------------------------------------------------------------------
// 연결
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::ConnectAndSubscribe()
{
	FMQTTURL Url;
	Url.Host   = BrokerHost;
	Url.Port   = BrokerPort;
	Url.Scheme = EMQTTScheme::MQTT;

	UE_LOG(LogMqttVehicle, Log, TEXT("MQTT vehicle bridge connecting to %s:%d ..."),
		*BrokerHost, BrokerPort);

	Client = IMQTTCoreModule::Get().GetOrCreateClient(Url);
	if (!Client.IsValid())
	{
		UE_LOG(LogMqttVehicle, Error,
			TEXT("MQTT vehicle bridge: failed to create client for %s:%d."),
			*BrokerHost, BrokerPort);
		return;
	}

	// 이 클라이언트가 받는 모든 메시지가 여기로 온다. 토픽 필터링은 핸들러에서 한다.
	MessageHandle = Client->OnMessage().AddUObject(this, &UMqttVehicleBridgeSubsystem::HandleMqttMessage);

	TWeakObjectPtr<UMqttVehicleBridgeSubsystem> WeakThis(this);

	Client->Connect().Next([WeakThis](EMQTTConnectReturnCode InCode)
	{
		// Connect() 의 continuation 은 임의의 스레드에서 실행된다.
		// UObject 를 만지기 전에 반드시 게임 스레드로 넘긴다.
		AsyncTask(ENamedThreads::GameThread, [WeakThis, InCode]()
		{
			UMqttVehicleBridgeSubsystem* Self = WeakThis.Get();
			if (!Self || !Self->Client.IsValid())
			{
				return;
			}

			UE_LOG(LogMqttVehicle, Log, TEXT("MQTT connect returned code %d"),
				static_cast<int32>(InCode));

			Self->bConnected = true;

			// QoS 0. 100ms 마다 갱신되는 제어 스트림이라 재전송이 의미 없다.
			TArray<TPair<FString, EMQTTQualityOfService>> Subscriptions;
			Subscriptions.Emplace(Self->CmdTopic,      EMQTTQualityOfService::Once);
			Subscriptions.Emplace(Self->AlertTopic,    EMQTTQualityOfService::Once);
			Subscriptions.Emplace(Self->DispatchTopic, EMQTTQualityOfService::Once);
			Self->Client->Subscribe(Subscriptions);

			// 이 로그가 보이면 연결 성공이다. 검증 항목.
			UE_LOG(LogMqttVehicle, Log, TEXT("MQTT vehicle bridge subscribed to %s, %s, %s"),
				*Self->CmdTopic, *Self->AlertTopic, *Self->DispatchTopic);
		});
	});
}

// ----------------------------------------------------------------------------
// 발행 (Blueprint 에서 호출, 게임 스레드)
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::PublishString(const FString& Topic, const FString& Payload)
{
	if (!Client.IsValid() || !bConnected)
	{
		UE_LOG(LogMqttVehicle, Warning,
			TEXT("Publish skipped (not connected): %s"), *Topic);
		return;
	}

	Client->Publish(Topic, Payload, EMQTTQualityOfService::Once, /*bRetain=*/false);
	UE_LOG(LogMqttVehicle, Verbose, TEXT("PUBLISH %s : %s"), *Topic, *Payload);
}

void UMqttVehicleBridgeSubsystem::PublishHomeTrigger(const FString& TriggerType, const FString& Room)
{
	const FString Payload = FString::Printf(
		TEXT("{\"type\":\"%s\",\"room\":\"%s\"}"),
		*TriggerType, *Room);

	UE_LOG(LogMqttVehicle, Log, TEXT("HOME TRIGGER -> %s"), *Payload);
	PublishString(TriggerTopic, Payload);
}

void UMqttVehicleBridgeSubsystem::PublishDispatch(const FString& Action)
{
	const FString Payload = FString::Printf(TEXT("{\"action\":\"%s\"}"), *Action);

	UE_LOG(LogMqttVehicle, Log, TEXT("DISPATCH -> %s"), *Payload);
	PublishString(DispatchTopic, Payload);
}

// ----------------------------------------------------------------------------
// 수신 (게임 스레드 아님)
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::HandleMqttMessage(const FMQTTClientMessage& InMessage)
{
	const bool bIsCmd      = (InMessage.Topic == CmdTopic);
	const bool bIsAlert    = (InMessage.Topic == AlertTopic);
	const bool bIsDispatch = (InMessage.Topic == DispatchTopic);
	if (!bIsCmd && !bIsAlert && !bIsDispatch)
	{
		return;
	}

	// Payload 는 TArray<uint8> 이다. UTF-8 로 직접 디코드한다.
	const FUTF8ToTCHAR Converted(
		reinterpret_cast<const ANSICHAR*>(InMessage.Payload.GetData()),
		InMessage.Payload.Num());
	const FString Json(Converted.Length(), Converted.Get());

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogMqttVehicle, Warning, TEXT("MQTT vehicle bridge: bad JSON payload: %s"), *Json);
		return;
	}

	if (bIsCmd)
	{
		double Th = 0.0;
		double St = 0.0;
		Root->TryGetNumberField(TEXT("th"), Th);
		Root->TryGetNumberField(TEXT("st"), St);

		FScopeLock Lock(&StateLock);
		CmdThrottle        = FMath::Clamp(static_cast<float>(Th), -1.0f, 1.0f);
		CmdSteering        = FMath::Clamp(static_cast<float>(St), -1.0f, 1.0f);
		LastMessageSeconds = FPlatformTime::Seconds();
		return;
	}

	if (bIsDispatch)
	{
		// --- 관제의 출동 지령 ---
		// 앰뷸런스가 실제로 움직이는 계기는 경보가 아니라 이 지령이다.
		FString Action;
		Root->TryGetStringField(TEXT("action"), Action);

		FScopeLock Lock(&StateLock);
		PendingDispatchAction = Action;
		bPendingDispatch      = true;
		return;
	}

	// --- 스마트홈 경보 ---
	// 여기는 게임 스레드가 아니므로 UPROPERTY 를 만지거나 델리게이트를 쏘면 안 된다.
	// 담아두기만 하고 Tick 에서 처리한다.
	FString Type = TEXT("normal");
	FString Room;
	double  Level = 0.0;
	Root->TryGetStringField(TEXT("type"), Type);
	Root->TryGetStringField(TEXT("room"), Room);
	Root->TryGetNumberField(TEXT("level"), Level);

	FScopeLock Lock(&StateLock);
	PendingAlertType  = Type;
	PendingAlertRoom  = Room;
	PendingAlertLevel = static_cast<int32>(Level);
	bPendingAlert     = true;
}

// ----------------------------------------------------------------------------
// Tick
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bConnected)
	{
		return;
	}

	// 통신 스레드에서 담아둔 경보를 게임 스레드에서 처리한다.
	bool bHasAlert = false;
	FString Type, Room;
	int32 Level = 0;

	bool bHasDispatch = false;
	FString Action;
	{
		FScopeLock Lock(&StateLock);
		if (bPendingAlert)
		{
			bHasAlert     = true;
			bPendingAlert = false;
			Type  = PendingAlertType;
			Room  = PendingAlertRoom;
			Level = PendingAlertLevel;
		}
		if (bPendingDispatch)
		{
			bHasDispatch     = true;
			bPendingDispatch = false;
			Action = PendingDispatchAction;
		}
	}

	if (bHasAlert)
	{
		AlertType  = Type;
		AlertRoom  = Room;
		AlertLevel = Level;

		UE_LOG(LogMqttVehicle, Log, TEXT("HOME ALERT: %s @ %s (level %d)"), *Type, *Room, Level);
		OnHomeAlert.Broadcast(Type, Room, Level);
	}

	if (bHasDispatch)
	{
		DispatchAction = Action;

		UE_LOG(LogMqttVehicle, Log, TEXT("DISPATCH: %s"), *Action);
		OnDispatch.Broadcast(Action);
	}

	ApplyCommandToVehicle();
}

UChaosWheeledVehicleMovementComponent* UMqttVehicleBridgeSubsystem::FindMovementComponent() const
{
	const UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UChaosWheeledVehicleMovementComponent>() : nullptr;
}

void UMqttVehicleBridgeSubsystem::ApplyCommandToVehicle()
{
	float Th = 0.0f;
	float St = 0.0f;
	double LastMessage = 0.0;
	{
		FScopeLock Lock(&StateLock);
		Th = CmdThrottle;
		St = CmdSteering;
		LastMessage = LastMessageSeconds;
	}

	// 명령이 한 번도 온 적이 없으면 차량 입력에 전혀 개입하지 않는다.
	// (Play 창에서 키보드로 직접 운전하는 기본 동작을 그대로 두기 위함)
	if (LastMessage <= 0.0)
	{
		return;
	}

	const double Elapsed = FPlatformTime::Seconds() - LastMessage;

	// 오래 조용하면 제어권을 놓는다.
	if (Elapsed > ControlReleaseAfter)
	{
		FScopeLock Lock(&StateLock);
		LastMessageSeconds = 0.0;
		CmdThrottle = 0.0f;
		CmdSteering = 0.0f;
		return;
	}

	UChaosWheeledVehicleMovementComponent* Movement = FindMovementComponent();
	if (!Movement)
	{
		return;
	}

	// 페일세이프: 명령이 끊기면 정지시킨다.
	// 스로틀만 0으로 두면 관성으로 계속 굴러가므로 브레이크까지 건다.
	if (Elapsed > InputTimeout)
	{
		Movement->SetThrottleInput(0.0f);
		Movement->SetSteeringInput(0.0f);
		Movement->SetBrakeInput(1.0f);
		return;
	}

	// Chaos 의 SetThrottleInput 은 0..1 로 클램프되므로 th 를 그대로 넣으면
	// 음수(후진)가 사라진다. 전진/후진은 기어로, 감속은 브레이크로 나눈다.
	const float ForwardSpeed = Movement->GetForwardSpeed();   // cm/s
	float Throttle = 0.0f;
	float Brake = 0.0f;

	if (Th > 0.05f)
	{
		if (ForwardSpeed < -StopSpeedThreshold)
		{
			Brake = 1.0f;                        // 후진 중 -> 먼저 제동
		}
		else
		{
			Movement->SetTargetGear(1, true);    // 전진 기어
			Throttle = Th;
		}
	}
	else if (Th < -0.05f)
	{
		if (ForwardSpeed > StopSpeedThreshold)
		{
			Brake = 1.0f;                        // 전진 중 -> 먼저 제동
		}
		else
		{
			Movement->SetTargetGear(-1, true);   // 후진 기어
			Throttle = -Th;
		}
	}
	else
	{
		// 명령이 0이면 브레이크로 붙잡는다.
		//  - 스로틀만 0으로 두면 경사에서 계속 굴러간다
		//  - 후진 기어가 남아 있으면 아이들 토크로 뒤로 밀린다 (오토 차량의 크리프)
		// 로버는 th=0 이면 모터가 즉시 0이 되어 그 자리에 서므로, 그 동작에 맞춘다.
		Brake = 1.0f;

		if (FMath::Abs(ForwardSpeed) < StopSpeedThreshold)
		{
			Movement->SetTargetGear(1, true);    // 멈췄으면 기어를 전진으로 되돌린다
		}
	}

	Movement->SetThrottleInput(Throttle);
	Movement->SetBrakeInput(Brake);
	Movement->SetSteeringInput(St);
}
