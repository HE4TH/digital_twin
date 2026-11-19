// Unreal manual vehicle input -> MQTT. Configure one receiver protocol in DefaultGame.ini.
// Legacy incoming vehicle control is opt-in. Home APIs are inert compatibility stubs.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SharedPointer.h"
#include "MqttVehicleBridgeSubsystem.generated.h"

class IMQTTClient;
class UChaosWheeledVehicleMovementComponent;
class AMqttVehiclePawn;
struct FMQTTClientMessage;

/**
 * Archived Blueprint signature only. The exploration bridge never broadcasts home alerts.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHomeAlert,
	const FString&, AlertType, const FString&, Room, int32, Level);

UCLASS(Config = Game, BlueprintType)
class MQTTVEHICLE_API UMqttVehicleBridgeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---- 스마트홈 경보 (Blueprint 에서 사용) -----------------------------

	/** Compatibility event for archived maps; never broadcast by this subsystem. */
	UPROPERTY(BlueprintAssignable, Category = "MQTT|Home")
	FOnHomeAlert OnHomeAlert;

	/** Compatibility state, kept at normal. */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	FString AlertType = TEXT("normal");

	/** Compatibility state, kept empty. */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	FString AlertRoom;

	/** Compatibility state, kept at zero. */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	int32 AlertLevel = 0;

	/** Retained config name; no emergency topic subscription. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString AlertTopic = TEXT("home/alert");

	/** Retained config name; PublishHomeTrigger no longer publishes. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString TriggerTopic = TEXT("home/trigger");

	// ---- 발행 (Blueprint 에서 호출) ---------------------------------------

	/** Compatibility stub: intentionally does nothing. Use exploration events in the controller. */
	UFUNCTION(BlueprintCallable, Category = "MQTT|Home", meta=(DeprecatedFunction, DeprecationMessage="Emergency scenario retired; use exploration events."))
	void PublishHomeTrigger(const FString& TriggerType, const FString& Room);

	/** 임의의 토픽으로 문자열을 발행한다. 게임 스레드에서 호출할 것. */
	UFUNCTION(BlueprintCallable, Category = "MQTT")
	void PublishString(const FString& Topic, const FString& Payload);

	/** 현재 브로커에 연결되어 있는가. */
	UFUNCTION(BlueprintPure, Category = "MQTT")
	bool IsBrokerConnected() const;

 // An offline virtual experience is allowed only while physical output is disabled.
 bool IsLocalReturnConnectionReady() const
 {
  return !bReceiveVehicleCommands && (!bPublishVehicleInput || !bDashboardOutputEnabled || IsBrokerConnected());
 }

	// ---------------------------------------------------------------------

	//~ UWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }

	// ---- ini 로 바꿀 수 있는 설정 ----------------------------------------

	/** 라즈베리파이(브로커) IP. 학생마다 다르므로 반드시 확인할 것. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString BrokerHost = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	int32 BrokerPort = 1883;

	/** 조종 명령 토픽. 로버와 반드시 같아야 한다. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString CmdTopic = TEXT("veh/cmd");

	/** Keep off until the broker and the physical receiver have been checked. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Vehicle Output")
	bool bPublishVehicleInput = false;

	// Session-only dashboard gate: keep MQTT alive and send zero while disabled.
	bool bDashboardOutputEnabled = true;

	/** Explicit fixed-chassis return mirroring; does not guarantee real path/angle. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Vehicle Output")
	bool bPublishAutomaticReturn = false;

	/** veh/cmd uses th/st; car/control uses throttle/steering. Only one topic is sent. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Vehicle Output")
	FString ThrottleField = TEXT("th");

	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Vehicle Output")
	FString SteeringField = TEXT("st");

	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Vehicle Output", meta = (ClampMin = "0.02", ClampMax = "0.1"))
	float PublishInterval = 0.1f;

	/** Legacy web -> Unreal mode. Cannot be combined with vehicle publishing. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Legacy")
	bool bReceiveVehicleCommands = false;

	/** Retired config setting. Ignored even when true; retained for old config/Blueprint compatibility. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT|Legacy")
	bool bEnableHomeEvents = false;

	UPROPERTY(Config, EditAnywhere, Category = "MQTT", meta = (ClampMin = "0.5"))
	float ReconnectInterval = 2.0f;

	/** 이 시간(초) 동안 명령이 없으면 정지시킨다. 로버 쪽 워치독과 같은 값. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	float InputTimeout = 0.3f;

	/** 이 시간(초) 이상 조용하면 차량 제어권을 놓는다 (키보드로 직접 운전 가능). */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	float ControlReleaseAfter = 3.0f;

	// ---------------------------------------------------------------------

private:
	void ConnectAndSubscribe();
	void ReleaseClient();
	void ScheduleReconnect(const TCHAR* Reason);
	void UpdateVehicleOutput(double Now);
	void SendVehicleControl(const FVector2D& Input);
	AMqttVehiclePawn* FindLocalVehicle() const;

	/** MQTT 콜백을 게임 스레드로 전달한 뒤 호출한다. 적용은 Tick 에서 한다. */
	void HandleMqttMessage(const FMQTTClientMessage& InMessage);

	void ApplyCommandToVehicle();
	UChaosWheeledVehicleMovementComponent* FindMovementComponent() const;

	/** GetOrCreateClient 가 ESPMode::ThreadSafe 로 반환하므로 여기도 맞춰야 한다. */
	TSharedPtr<IMQTTClient, ESPMode::ThreadSafe> Client;
	FDelegateHandle MessageHandle;

	// ---- 스레드 경계: StateLock 아래에서만 접근 ----
	mutable FCriticalSection StateLock;
	float CmdThrottle = 0.0f;
	float CmdSteering = 0.0f;
	double LastMessageSeconds = 0.0;   // 0 == 아직 한 번도 수신 안 함
	// ----------------------------------------------

	bool bConnected = false;
	bool bInitialized = false;
	bool bConnecting = false;
	bool bPublishPending = false;
	bool bPingPending = false;
	uint32 ConnectionGeneration = 0;
	double ConnectStartedAt = 0.0;
	double NextReconnectAt = 0.0;
	double LastPublishAt = 0.0;
	double NextPingAt = 0.0;
	FVector2D LastPublishedInput = FVector2D::ZeroVector;
	TWeakObjectPtr<AMqttVehiclePawn> LastOutputPawn;

	/** 이 속도(cm/s) 아래면 정지로 보고 기어를 바꾼다. */
	static constexpr float StopSpeedThreshold = 50.0f;
};
