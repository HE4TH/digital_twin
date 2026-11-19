#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExplorationRoute.h"
#include "ExplorationDangerZone.h"
#include "UnderwaterPathPlanner.h"
#include "ExplorationReturnComponent.generated.h"

class AMqttVehiclePawn;
class UWhaleObservationComponent;

UENUM(BlueprintType)
enum class EExplorationReturnMode : uint8
{
	Unavailable, Manual, PreparingReturn, Returning, Arrived, Stopped, Planning
};

UENUM(BlueprintType)
enum class EExplorationReturnReason : uint8
{
	UserRequest, WhaleDiscovery, DangerZone
};

USTRUCT(BlueprintType)
struct FExplorationReturnSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	EExplorationReturnMode Mode = EExplorationReturnMode::Unavailable;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	FText Detail;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	EExplorationReturnReason Reason = EExplorationReturnReason::UserRequest;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	bool bInsideDangerZone = false;
	/** Includes quick crossings that end outside the volume. */
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	int32 DangerEncounterSerial = 0;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	float SecondsUntilReturn = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	float RemainingMeters = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	int32 RecordedPoints = 0;
	UPROPERTY(BlueprintReadOnly, Category="Exploration")
	bool bCanReturn = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExplorationReturnStateChanged, const FExplorationReturnSnapshot&, State);

/** Local Unreal route return only. Autonomous commands are excluded from vehicle MQTT output. */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class MQTTVEHICLE_API UExplorationReturnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UExplorationReturnComponent();
	UPROPERTY(BlueprintAssignable, Category="Exploration")
	FExplorationReturnStateChanged OnReturnStateChanged;
	UFUNCTION(BlueprintCallable, Category="Exploration")
	void RequestReturn();
	UFUNCTION(BlueprintCallable, Category="Exploration")
	void StopExploration();
	UFUNCTION(BlueprintCallable, Category="Exploration")
	void ResumeManual();
	UFUNCTION(BlueprintPure, Category="Exploration")
	FExplorationReturnSnapshot GetReturnState() const { return State; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	friend class FExplorationUnderwaterReturnTest;
	friend class FExplorationPlannedReturnTest;
	friend class FExplorationSceneReturnTest;
	UFUNCTION()
	void OnWhaleDiscovered(AActor* Whale);
	UFUNCTION()
	void OnObservationZoneChanged(bool bInside);
	void OnVehicleInterrupted(bool bManualOverride);
	void SetVehicle(AMqttVehiclePawn* NewVehicle);
	void RefreshHome();
	void RefreshDangerZones();
	void UpdateDangerZones();
	void StartReturn(EExplorationReturnReason Reason);
	void RecordPose(bool bForce = false);
	void FollowRoute(float DeltaTime);
	bool BeginTurnaround();
	void FollowTurnaround(float DeltaTime);
	bool bTurningInPlace = false;
	bool bArrivalTurn = false;
	float TurnRemainingDegrees = 0.0f;
	float TurnSeconds = 0.0f;
	FVector TurnStartPosition = FVector::ZeroVector;
	void StopWithReason(const FText& Reason, bool bInvalidateRoute = false);
	void LeaveAutomaticControl(bool bRequireRelease);
	void PublishState();
	bool IsAutomatic() const;
	bool CanDriveNow() const;
	FText GetControlBlockReason() const;

 void BeginNavigationPlan(bool bReplan = false);
 void TickNavigationPlan(float DeltaTime);
 void FollowNavigationPath(float DeltaTime);
 bool IsNavigationSegmentClear(const FVector& From, const FVector& To) const;
 float NavigationRemaining() const;
 FUnderwaterPathPlanner Navigator;
 TArray<FVector> NavigationPath;
 FVector NavigationHome = FVector::ZeroVector;
 FVector NavigationStart = FVector::ZeroVector;
 bool bNavigationHomeValid = false;
 bool bNavigationReturn = false;
 int32 NavigationIndex = 0;
 int32 ReplanCount = 0;
 float PlanningSeconds = 0;

	FExplorationRoute Route;
	FExplorationReturnSnapshot State;
	TMap<TWeakObjectPtr<AActor>, FExplorationDangerZoneState> DangerZones;
	TWeakObjectPtr<AMqttVehiclePawn> Vehicle;
	TWeakObjectPtr<AActor> HomeAnchor;
	TWeakObjectPtr<UWhaleObservationComponent> Observation;
	FDelegateHandle InterruptionHandle;
	FVector HomePosition = FVector::ZeroVector;
	float BestRemainingDistance = 0.0f;
	uint32 VehicleResetRevision = 0;
	int32 TargetIndex = INDEX_NONE;
	int32 SwimAimIndex = INDEX_NONE;
	float Countdown = 0.0f;
	float StoppedSeconds = 0.0f;
	float NoProgressSeconds = 0.0f;
	float PreparationSeconds = 0.0f;
	float SampleAccumulator = 0.0f;
	float StateAccumulator = 0.0f;
	double NextHomeSearch = 0.0;
	bool bDiscoveryHandled = false;
	bool bDiscoveryPending = false;
	uint64 LastUserInterruptionFrame = MAX_uint64;
	bool bActive = false;
};
