// Vehicle command publishing and optional legacy subscriptions, Unreal 5.7 MQTT API.

#include "MqttVehicleBridgeSubsystem.h"
#include "MqttVehicle.h"
#include "MqttVehiclePawn.h"

#include "Async/Async.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
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
	// Retain serialized Blueprint names, but retire the old emergency scenario in this module.
	if (bEnableHomeEvents)
	{
		UE_LOG(LogMqttVehicle, Warning, TEXT("Home events are retired; bEnableHomeEvents is ignored."));
	}
	bEnableHomeEvents = false;
	AlertType = TEXT("normal");
	AlertRoom.Reset();
	AlertLevel = 0;
	// An offline exploration session should not contact the configured physical-car broker.
	if (!bPublishVehicleInput && !bReceiveVehicleCommands)
	{
		UE_LOG(LogMqttVehicle, Log, TEXT("MQTT vehicle bridge disabled: offline exploration."));
		return;
	}

	if ((bPublishVehicleInput && bReceiveVehicleCommands) || BrokerHost.IsEmpty()
		|| BrokerPort < 1 || BrokerPort > 65535 || !FMQTTTopic::IsValid(CmdTopic)
		|| ThrottleField.IsEmpty() || SteeringField.IsEmpty() || ThrottleField == SteeringField)
	{
		UE_LOG(LogMqttVehicle, Error, TEXT("MQTT configuration invalid: check broker/topic/fields; output and legacy input are mutually exclusive."));
		return;
	}
	PublishInterval = FMath::Clamp(PublishInterval, 0.02f, 0.1f);
	ReconnectInterval = FMath::Max(ReconnectInterval, 0.5f);
	bInitialized = true;
	ConnectAndSubscribe();
}

void UMqttVehicleBridgeSubsystem::Deinitialize()
{
	if (AMqttVehiclePawn* Pawn = FindLocalVehicle())
	{
		Pawn->StopVehicleInput();
	}
	// Keep the closing client alive until the final stop has left its queue.
	// QoS 0 cannot confirm motor stop; the receiver still needs its own watchdog.
	if (bPublishVehicleInput && IsBrokerConnected())
	{
		const auto StopObject = MakeShared<FJsonObject>();
		StopObject->SetNumberField(ThrottleField, 0.0);
		StopObject->SetNumberField(SteeringField, 0.0);
		FString Stop;
		FJsonSerializer::Serialize(StopObject, TJsonWriterFactory<>::Create(&Stop));
		const auto ClosingClient = Client;
		ClosingClient->Publish(CmdTopic, Stop, EMQTTQualityOfService::Once, false)
			.Next([ClosingClient](bool)
			{
				AsyncTask(ENamedThreads::GameThread, [ClosingClient]() { ClosingClient->Disconnect(); });
			});
	}
	bInitialized = false;
	ReleaseClient();
	UE_LOG(LogMqttVehicle, Log, TEXT("MQTT vehicle bridge shut down."));
	Super::Deinitialize();
}

void UMqttVehicleBridgeSubsystem::ReleaseClient()
{
	++ConnectionGeneration;
	if (Client.IsValid())
	{
		if (MessageHandle.IsValid())
		{
			Client->OnMessage().Remove(MessageHandle);
			MessageHandle.Reset();
		}
		Client.Reset();
	}

	bConnected = false;
	bConnecting = false;
	bPublishPending = false;
	bPingPending = false;
}

void UMqttVehicleBridgeSubsystem::ScheduleReconnect(const TCHAR* Reason)
{
	UE_LOG(LogMqttVehicle, Warning, TEXT("MQTT disconnected: %s; retry in %.1f seconds."), Reason, ReconnectInterval);
	if (AMqttVehiclePawn* Pawn = FindLocalVehicle(); Pawn && bConnected && bPublishVehicleInput && bDashboardOutputEnabled)
	{
		Pawn->StopVehicleInput();
	}
	ReleaseClient();
	NextReconnectAt = FPlatformTime::Seconds() + ReconnectInterval;
}

bool UMqttVehicleBridgeSubsystem::IsBrokerConnected() const
{
	return bInitialized && bConnected && Client.IsValid() && Client->IsConnected();
}

TStatId UMqttVehicleBridgeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMqttVehicleBridgeSubsystem, STATGROUP_Tickables);
}

bool UMqttVehicleBridgeSubsystem::IsTickable() const
{
	return bInitialized && !IsTemplate();
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

	// Recreate after a failure so queued drive commands never replay on reconnect.
	Client = IMQTTCoreModule::Get().GetOrCreateClient(Url, true);
	if (!Client.IsValid())
	{
		UE_LOG(LogMqttVehicle, Error,
			TEXT("MQTT vehicle bridge: failed to create client for %s:%d."),
			*BrokerHost, BrokerPort);
		NextReconnectAt = FPlatformTime::Seconds() + ReconnectInterval;
		return;
	}

	TWeakObjectPtr<UMqttVehicleBridgeSubsystem> WeakThis(this);
	const uint32 Generation = ++ConnectionGeneration;
	bConnecting = true;
	ConnectStartedAt = FPlatformTime::Seconds();
	MessageHandle = Client->OnMessage().AddLambda([WeakThis, Generation](const FMQTTClientMessage& Message)
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Generation, Message]()
		{
			if (auto* Self = WeakThis.Get(); Self && Self->bInitialized && Self->ConnectionGeneration == Generation)
			{
				Self->HandleMqttMessage(Message);
			}
		});
	});

	Client->Connect().Next([WeakThis, Generation](EMQTTConnectReturnCode InCode)
	{
		// Connect() 의 continuation 은 임의의 스레드에서 실행된다.
		// UObject 를 만지기 전에 반드시 게임 스레드로 넘긴다.
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Generation, InCode]()
		{
			UMqttVehicleBridgeSubsystem* Self = WeakThis.Get();
			if (!Self || !Self->bInitialized || Self->ConnectionGeneration != Generation || !Self->Client.IsValid())
			{
				return;
			}

			Self->bConnecting = false;
			if ((InCode != EMQTTConnectReturnCode::Accepted && InCode != EMQTTConnectReturnCode::AlreadyConnected)
				|| !Self->Client->IsConnected())
			{
				UE_LOG(LogMqttVehicle, Warning, TEXT("MQTT connection failed, code %d."), static_cast<int32>(InCode));
				Self->ScheduleReconnect(TEXT("connection was not accepted"));
				return;
			}
			Self->bConnected = true;
			Self->NextPingAt = FPlatformTime::Seconds() + 2.0;
			Self->LastPublishAt = 0.0;
			Self->LastOutputPawn.Reset();
			if (auto* Pawn = Self->FindLocalVehicle(); Pawn && Self->bPublishVehicleInput && Self->bDashboardOutputEnabled)
			{
				Pawn->StopVehicleInput();
			}
			UE_LOG(LogMqttVehicle, Log, TEXT("MQTT connected; vehicle output=%d, legacy input=%d; home events retired."),
				Self->bPublishVehicleInput, Self->bReceiveVehicleCommands);
			UE_LOG(LogMqttVehicle, Log, TEXT("MQTT command destination: %s:%d topic=%s"),
				*Self->BrokerHost, Self->BrokerPort, *Self->CmdTopic);

			// QoS 0. 100ms 마다 갱신되는 제어 스트림이라 재전송이 의미 없다.
			TArray<TPair<FString, EMQTTQualityOfService>> Subscriptions;
			if (Self->bReceiveVehicleCommands)
			{
				Subscriptions.Emplace(Self->CmdTopic, EMQTTQualityOfService::Once);
			}
			if (Subscriptions.IsEmpty())
			{
				return;
			}
			Self->Client->Subscribe(Subscriptions).Next([WeakThis, Generation, Count = Subscriptions.Num()](const TArray<FMQTTSubscribeResult>& Results)
			{
				bool bSuccess = Results.Num() == Count;
				for (const auto& Result : Results)
				{
					bSuccess &= Result.ReturnCode != EMQTTSubscribeReturnCode::Failure && Result.Subscription.IsValid();
				}
				AsyncTask(ENamedThreads::GameThread, [WeakThis, Generation, bSuccess, Count]()
				{
					if (auto* Bridge = WeakThis.Get(); Bridge && Bridge->bInitialized && Bridge->ConnectionGeneration == Generation)
					{
						if (!bSuccess)
						{
							Bridge->ScheduleReconnect(TEXT("subscription rejected"));
							return;
						}
						UE_LOG(LogMqttVehicle, Log, TEXT("MQTT confirmed %d subscriptions."), Count);
					}
				});
			});
		});
	});
}

// ----------------------------------------------------------------------------
// 발행 (Blueprint 에서 호출, 게임 스레드)
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::PublishString(const FString& Topic, const FString& Payload)
{
	if (!IsBrokerConnected())
	{
		UE_LOG(LogMqttVehicle, Warning,
			TEXT("Publish skipped (not connected): %s"), *Topic);
		return;
	}

	// The engine's FString overload counts TCHARs rather than UTF-8 bytes.
	const FTCHARToUTF8 Utf8(*Payload);
	const TArray<uint8> Bytes(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	Client->Publish(Topic, Bytes, EMQTTQualityOfService::Once, false).Next([Topic](bool bSuccess)
	{
		if (!bSuccess)
		{
			UE_LOG(LogMqttVehicle, Warning, TEXT("MQTT publish failed: %s"), *Topic);
		}
	});
	UE_LOG(LogMqttVehicle, Verbose, TEXT("PUBLISH %s : %s"), *Topic, *Payload);
}

void UMqttVehicleBridgeSubsystem::PublishHomeTrigger(const FString& TriggerType, const FString& Room)
{
	// Compatibility stub for archived Rescue Blueprint nodes. Never sends a home trigger.
	UE_LOG(LogMqttVehicle, Verbose, TEXT("PublishHomeTrigger ignored: emergency scenario retired."));
}

// ----------------------------------------------------------------------------
// 수신 (콜백에서 게임 스레드로 전달된 메시지)
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::HandleMqttMessage(const FMQTTClientMessage& InMessage)
{
	const bool bIsCmd   = bReceiveVehicleCommands && !bPublishVehicleInput && (InMessage.Topic == CmdTopic);
	if (!bIsCmd)
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
		if (InMessage.bRetain || !Root->TryGetNumberField(TEXT("th"), Th)
			|| !Root->TryGetNumberField(TEXT("st"), St) || !FMath::IsFinite(Th) || !FMath::IsFinite(St))
		{
			return;
		}

		FScopeLock Lock(&StateLock);
		CmdThrottle        = FMath::Clamp(static_cast<float>(Th), -1.0f, 1.0f);
		CmdSteering        = FMath::Clamp(static_cast<float>(St), -1.0f, 1.0f);
		LastMessageSeconds = FPlatformTime::Seconds();
		return;
	}

}

// ----------------------------------------------------------------------------
// Tick
// ----------------------------------------------------------------------------

void UMqttVehicleBridgeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const double Now = FPlatformTime::Seconds();
	// The legacy watchdog must keep running after a network disconnect.
	if (bReceiveVehicleCommands && !bPublishVehicleInput)
	{
		ApplyCommandToVehicle();
	}
	if (bConnected && !IsBrokerConnected())
	{
		ScheduleReconnect(TEXT("socket disconnected"));
	}
	if (bConnecting && Now - ConnectStartedAt > 15.0)
	{
		ScheduleReconnect(TEXT("connect timed out"));
	}
	if (!bConnected)
	{
		if (!bConnecting && Now >= NextReconnectAt)
		{
			ConnectAndSubscribe();
		}
		return;
	}
	if ((bPublishPending && Now - LastPublishAt > 0.5) || (bPingPending && Now > NextPingAt + 2.0))
	{
		ScheduleReconnect(TEXT("publish or ping timed out"));
		return;
	}
	if (!bPingPending && Now >= NextPingAt)
	{
		bPingPending = true;
		NextPingAt = Now + 2.0;
		TWeakObjectPtr<UMqttVehicleBridgeSubsystem> WeakThis(this);
		const uint32 Generation = ConnectionGeneration;
		Client->Ping(2.0f).Next([WeakThis, Generation](bool bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Generation, bSuccess]()
			{
				if (auto* Self = WeakThis.Get(); Self && Self->bInitialized && Self->ConnectionGeneration == Generation)
				{
					Self->bPingPending = false;
					if (!bSuccess)
					{
						Self->ScheduleReconnect(TEXT("broker ping failed"));
					}
				}
			});
		});
	}
	if (bPublishVehicleInput)
	{
		UpdateVehicleOutput(Now);
	}

}

AMqttVehiclePawn* UMqttVehicleBridgeSubsystem::FindLocalVehicle() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	return PC && PC->IsLocalController() ? Cast<AMqttVehiclePawn>(PC->GetPawn()) : nullptr;
}

void UMqttVehicleBridgeSubsystem::UpdateVehicleOutput(double Now)
{
	AMqttVehiclePawn* Pawn = FindLocalVehicle();
	const UWorld* World = GetWorld();
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const bool bCanDrive = Pawn && PC && !PC->IsMoveInputIgnored() && !World->IsPaused()
		&& MqttVehicleControl::HasFocus(Viewport && Viewport->Viewport && Viewport->Viewport->HasFocus());
	if (Pawn && (!bCanDrive || LastOutputPawn.Get() != Pawn))
	{
		Pawn->StopVehicleInput();
	}
	LastOutputPawn = Pawn;
	const FVector2D Input = (!bCanDrive || !bDashboardOutputEnabled) ? FVector2D::ZeroVector
		: (bPublishAutomaticReturn && Pawn->IsExplorationControlActive()
			? Pawn->GetAutomaticVehicleControlInput() : Pawn->GetVehicleControlInput());
	const bool bStopping = Input.IsNearlyZero() && !LastPublishedInput.IsNearlyZero();
	if (!bPublishPending && (bStopping || Now - LastPublishAt >= PublishInterval))
	{
		SendVehicleControl(Input);
	}
}

void UMqttVehicleBridgeSubsystem::SendVehicleControl(const FVector2D& Input)
{
	if (!IsBrokerConnected())
	{
		return;
	}
	const auto Object = MakeShared<FJsonObject>();
	Object->SetNumberField(ThrottleField, Input.X);
	Object->SetNumberField(SteeringField, Input.Y);
	FString Payload;
	FJsonSerializer::Serialize(Object, TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload));
	const FTCHARToUTF8 Utf8(*Payload);
	const TArray<uint8> Bytes(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	bPublishPending = true;
	LastPublishAt = FPlatformTime::Seconds();
	const bool bOutputChanged = !Input.Equals(LastPublishedInput, 0.05f);
	LastPublishedInput = Input;
	TWeakObjectPtr<UMqttVehicleBridgeSubsystem> WeakThis(this);
	const uint32 Generation = ConnectionGeneration;
	Client->Publish(CmdTopic, Bytes, EMQTTQualityOfService::Once, false).Next([WeakThis, Generation, bOutputChanged, Input](bool bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Generation, bSuccess, bOutputChanged, Input]()
		{
			if (auto* Self = WeakThis.Get(); Self && Self->bInitialized && Self->ConnectionGeneration == Generation)
			{
				Self->bPublishPending = false;
				if (!bSuccess)
				{
					Self->ScheduleReconnect(TEXT("vehicle publish failed"));
				}
				else if (bOutputChanged)
				{
					// QoS 0 completion is local send success, not receiver/motor confirmation.
					UE_LOG(LogMqttVehicle, Log, TEXT("MQTT command sent locally: topic=%s th=%.3f st=%.3f (receiver not confirmed)"),
						*Self->CmdTopic, Input.X, Input.Y);
				}
			}
		});
	});
	UE_LOG(LogMqttVehicle, Verbose, TEXT("VEHICLE OUTPUT %s : %s"), *CmdTopic, *Payload);
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

	// Legacy commands explicitly choose gear and split acceleration from braking.
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
