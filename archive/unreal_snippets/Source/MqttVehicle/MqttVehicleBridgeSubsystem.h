// MqttVehicleBridgeSubsystem.h  (v3 - UE 5.7 실제 MQTT API 기준)
//
// 라즈베리파이 브로커를 구독해서, 웹/휴대폰이 보내는 조종 명령으로
// Unreal 차량을 움직인다. 실물 로버와 같은 명령을 받으므로 둘이 동시에 움직인다.
//
//   구독 : veh/cmd       {"th": -1..1, "st": -1..1}   사람의 수동 조종
//            th = 전후 (throttle)   st = 좌우 (steering)
//          home/alert    {"type":..,"room":..,"level":0~2}   집에서 온 경보
//          veh/dispatch  {"action":"go"|"return"|"cancel"}   관제의 출동 승인
//   발행 : home/trigger  Unreal 안에서 상황이 벌어졌을 때 (PublishHomeTrigger)
//          veh/dispatch  Unreal 에서 출동을 승인할 때 (PublishDispatch)
//
// 브로커 주소는 Config/DefaultGame.ini 에서 바꾼다. 재빌드 필요 없음.
//
//   [/Script/MqttVehicle.MqttVehicleBridgeSubsystem]
//   BrokerHost=192.168.219.51
//   BrokerPort=1883
//
// 게임 월드(PIE / Standalone)에서만 동작하고 Editor 월드에서는 연결하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SharedPointer.h"
#include "MqttVehicleBridgeSubsystem.generated.h"

class IMQTTClient;
class UChaosWheeledVehicleMovementComponent;
struct FMQTTClientMessage;

/**
 * 집에서 이상 신호가 왔을 때 발생한다.
 *   AlertType : "normal" | "no_motion" | "fall"
 *   Level     : 0 정상 / 1 주의 / 2 비상
 * Blueprint 에서 이 이벤트에 바인딩해 앰뷸런스를 움직이거나 경고 UI를 띄우면 된다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHomeAlert,
	const FString&, AlertType, const FString&, Room, int32, Level);

/**
 * 관제(휴대폰)가 출동을 승인했을 때 발생한다.
 *   Action : "go" 출동 / "return" 복귀 / "cancel" 취소
 *
 * 경보(OnHomeAlert)는 "무슨 일이 일어났다"일 뿐이고,
 * 앰뷸런스가 실제로 움직이는 것은 이 이벤트다. 사람의 승인이 사이에 들어간다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDispatch, const FString&, Action);

UCLASS(Config = Game, BlueprintType)
class MQTTVEHICLE_API UMqttVehicleBridgeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---- 스마트홈 경보 (Blueprint 에서 사용) -----------------------------

	/** 상태가 바뀔 때마다 게임 스레드에서 브로드캐스트된다. */
	UPROPERTY(BlueprintAssignable, Category = "MQTT|Home")
	FOnHomeAlert OnHomeAlert;

	/** 현재 경보 종류. "normal" | "no_motion" | "fall" */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	FString AlertType = TEXT("normal");

	/** 이상이 감지된 방 이름. */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	FString AlertRoom;

	/** 0 정상 / 1 주의 / 2 비상 */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Home")
	int32 AlertLevel = 0;

	/** 스마트홈 경보 토픽. home_sim.py 와 같아야 한다. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString AlertTopic = TEXT("home/alert");

	/** 상황 발생 토픽. home_sim.py 가 이걸 구독한다. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString TriggerTopic = TEXT("home/trigger");

	// ---- 출동 지령 (Blueprint 에서 사용) -----------------------------------

	/** 출동 승인/복귀 지령이 왔을 때 게임 스레드에서 브로드캐스트된다. */
	UPROPERTY(BlueprintAssignable, Category = "MQTT|Dispatch")
	FOnDispatch OnDispatch;

	/** 마지막 지령. "go" | "return" | "cancel" | "" */
	UPROPERTY(BlueprintReadOnly, Category = "MQTT|Dispatch")
	FString DispatchAction;

	/** 출동 지령 토픽. rover_ambulance.py / home.html 과 같아야 한다. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString DispatchTopic = TEXT("veh/dispatch");

	/**
	 * Unreal 안에서 출동을 승인한다 (휴대폰 대신 쓸 수 있는 경로).
	 *   Action : "go" | "return" | "cancel"
	 */
	UFUNCTION(BlueprintCallable, Category = "MQTT|Dispatch")
	void PublishDispatch(const FString& Action);

	// ---- 발행 (Blueprint 에서 호출) ---------------------------------------

	/**
	 * 가상 세계에서 상황을 발생시킨다.
	 *   TriggerType : "fall" | "no_motion" | "clear"
	 *   Room        : 예) "living"
	 *
	 * home_sim.py 가 이걸 받아 상태를 바꾸고 home/alert 를 발행한다.
	 * 그 결과 실물 로버와 Unreal 이 함께 반응한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MQTT|Home")
	void PublishHomeTrigger(const FString& TriggerType, const FString& Room);

	/** 임의의 토픽으로 문자열을 발행한다. 게임 스레드에서 호출할 것. */
	UFUNCTION(BlueprintCallable, Category = "MQTT")
	void PublishString(const FString& Topic, const FString& Payload);

	/** 현재 브로커에 연결되어 있는가. */
	UFUNCTION(BlueprintPure, Category = "MQTT")
	bool IsBrokerConnected() const { return bConnected; }

	// ---------------------------------------------------------------------

	//~ UWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	// ---- ini 로 바꿀 수 있는 설정 ----------------------------------------

	/** 라즈베리파이(브로커) IP. 학생마다 다르므로 반드시 확인할 것. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString BrokerHost = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	int32 BrokerPort = 1883;

	/** 조종 명령 토픽. 로버와 반드시 같아야 한다. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	FString CmdTopic = TEXT("veh/cmd");

	/** 이 시간(초) 동안 명령이 없으면 정지시킨다. 로버 쪽 워치독과 같은 값. */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	float InputTimeout = 0.3f;

	/** 이 시간(초) 이상 조용하면 차량 제어권을 놓는다 (키보드로 직접 운전 가능). */
	UPROPERTY(Config, EditAnywhere, Category = "MQTT")
	float ControlReleaseAfter = 3.0f;

	// ---------------------------------------------------------------------

private:
	void ConnectAndSubscribe();

	/** MQTT 수신 콜백. 게임 스레드가 아니다 — 값만 저장하고 적용은 Tick 에서 한다. */
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

	// 경보는 통신 스레드에서 도착하므로 여기 담아두고 Tick 에서 브로드캐스트한다.
	// (델리게이트 브로드캐스트와 UPROPERTY 갱신은 게임 스레드에서만 해야 한다)
	bool    bPendingAlert = false;
	FString PendingAlertType;
	FString PendingAlertRoom;
	int32   PendingAlertLevel = 0;

	bool    bPendingDispatch = false;
	FString PendingDispatchAction;

	/** 이 속도(cm/s) 아래면 정지로 보고 기어를 바꾼다. */
	static constexpr float StopSpeedThreshold = 50.0f;
};
