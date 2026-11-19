#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WhaleObservationComponent.generated.h"

/** Session progress only: this does not save a discovery to disk or control the vehicle. */
struct FWhaleObservationProgress
{
	bool Update(bool bTargetsAvailable, double HorizontalDistanceCm, double HeightDifferenceCm,
		double EnterRadiusCm, double ExitRadiusCm, double HeightToleranceCm);

	bool IsAvailable() const { return bAvailable; }
	bool IsInside() const { return bInside; }
	bool HasDiscoveredWhale() const { return bDiscovered; }

private:
	bool bAvailable = false;
	bool bInside = false;
	bool bDiscovered = false;
};

USTRUCT(BlueprintType)
struct FWhaleObservationSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Observation")
	bool bAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category="Observation")
	bool bInObservationZone = false;

	UPROPERTY(BlueprintReadOnly, Category="Observation")
	bool bWhaleDiscovered = false;

	UPROPERTY(BlueprintReadOnly, Category="Observation")
	bool bShowDiscoveryNotice = false;

	/** Horizontal distance to the pad centre, not distance to the whale. */
	UPROPERTY(BlueprintReadOnly, Category="Observation")
	float DistanceToPadMeters = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWhaleObservationStateChanged, const FWhaleObservationSnapshot&, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWhaleObservationZoneChanged, bool, bInside);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWhaleDiscovered, AActor*, Whale);

/** Detects the local explorer entering the tagged observation pad; no visibility/AI detection. */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class MQTTVEHICLE_API UWhaleObservationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWhaleObservationComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Observation")
	FName ObservationAnchorTag = TEXT("WhaleObservationAnchor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Observation")
	FName WhaleSubjectTag = TEXT("WhaleObservationSubject");

	/** The current pad has a 600 cm radius. Distances use the pawn origin. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Observation", meta=(ClampMin="1", Units="cm"))
	float EnterRadiusCm = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Observation", meta=(ClampMin="1", Units="cm"))
	float ExitRadiusCm = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Observation", meta=(ClampMin="1", Units="cm"))
	float HeightToleranceCm = 300.0f;

	UPROPERTY(BlueprintAssignable, Category="Observation")
	FWhaleObservationStateChanged OnObservationStateChanged;

	/** Repeated entry/exit signal, separate from the one-shot discovery. */
	UPROPERTY(BlueprintAssignable, Category="Observation")
	FWhaleObservationZoneChanged OnObservationZoneChanged;

	/** Once per controller lifetime. A future return controller can subscribe explicitly. */
	UPROPERTY(BlueprintAssignable, Category="Observation")
	FWhaleDiscovered OnWhaleDiscovered;

	UFUNCTION(BlueprintPure, Category="Observation")
	FWhaleObservationSnapshot GetObservationState() const { return State; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ResolveSceneActors();
	void UpdateObservation();

	TWeakObjectPtr<AActor> ObservationAnchor;
	TWeakObjectPtr<AActor> WhaleSubject;
	FWhaleObservationProgress Progress;
	FWhaleObservationSnapshot State;
	double NextActorSearchTime = 0.0;
	double DiscoveryNoticeUntil = 0.0;
	bool bObservationActive = false;
	bool bWarnedAmbiguousActors = false;
};
