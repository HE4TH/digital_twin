// MqttVehicleBridgeSubsystem.h  (v3 - UE 5.7 실제 MQTT API 기준)
//
// 라즈베리파이 브로커를 구독해서, 웹/휴대폰이 보내는 조종 명령으로
// Unreal 차량을 움직인다. 실물 로버와 같은 명령을 받으므로 둘이 동시에 움직인다.
//
//   구독 : veh/cmd    {"th": -1..1, "st": -1..1}
//            th = 전후 (throttle)   st = 좌우 (steering)
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

UCLASS(Config = Game)
class MQTTVEHICLE_API UMqttVehicleBridgeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
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

	/** 이 속도(cm/s) 아래면 정지로 보고 기어를 바꾼다. */
	static constexpr float StopSpeedThreshold = 50.0f;
};
