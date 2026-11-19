#include "WhaleObservationComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MqttVehicle.h"
#include "MqttVehiclePawn.h"

bool FWhaleObservationProgress::Update(bool bTargetsAvailable, double HorizontalDistanceCm,
	double HeightDifferenceCm, double EnterRadiusCm, double ExitRadiusCm, double HeightToleranceCm)
{
	bAvailable = bTargetsAvailable && FMath::IsFinite(HorizontalDistanceCm)
		&& FMath::IsFinite(HeightDifferenceCm) && FMath::IsFinite(EnterRadiusCm)
		&& FMath::IsFinite(ExitRadiusCm) && FMath::IsFinite(HeightToleranceCm)
		&& HorizontalDistanceCm >= 0.0 && EnterRadiusCm > 0.0
		&& ExitRadiusCm >= EnterRadiusCm && HeightToleranceCm > 0.0;
	const double RadiusCm = bInside ? ExitRadiusCm : EnterRadiusCm;
	bInside = bAvailable && HorizontalDistanceCm <= RadiusCm
		&& FMath::Abs(HeightDifferenceCm) <= HeightToleranceCm;
	const bool bFirstDiscovery = bInside && !bDiscovered;
	bDiscovered |= bFirstDiscovery;
	return bFirstDiscovery;
}

UWhaleObservationComponent::UWhaleObservationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UWhaleObservationComponent::BeginPlay()
{
	Super::BeginPlay();
	const APlayerController* Controller = Cast<APlayerController>(GetOwner());
	const UWorld* World = GetWorld();
	bObservationActive = Controller && Controller->IsLocalPlayerController() && World
		&& (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
	SetComponentTickEnabled(bObservationActive);
	if (bObservationActive)
	{
		UpdateObservation();
	}
}

void UWhaleObservationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Teardown is not a zone exit event; do not trigger gameplay during level shutdown.
	bObservationActive = false;
	ObservationAnchor.Reset();
	WhaleSubject.Reset();
	Super::EndPlay(EndPlayReason);
}

void UWhaleObservationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bObservationActive)
	{
		UpdateObservation();
	}
}

void UWhaleObservationComponent::ResolveSceneActors()
{
	TArray<AActor*> Anchors;
	TArray<AActor*> Whales;
	UGameplayStatics::GetAllActorsWithTag(this, ObservationAnchorTag, Anchors);
	UGameplayStatics::GetAllActorsWithTag(this, WhaleSubjectTag, Whales);
	ObservationAnchor.Reset();
	WhaleSubject.Reset();
	if (Anchors.Num() == 1 && Whales.Num() == 1 && Anchors[0] != Whales[0])
	{
		ObservationAnchor = Anchors[0];
		WhaleSubject = Whales[0];
	}
	const bool bAmbiguous = Anchors.Num() > 1 || Whales.Num() > 1
		|| (Anchors.Num() == 1 && Whales.Num() == 1 && Anchors[0] == Whales[0]);
	if (bAmbiguous && !bWarnedAmbiguousActors)
	{
		UE_LOG(LogMqttVehicle, Warning, TEXT("Whale observation requires one anchor and one distinct subject. Check actor tags."));
	}
	bWarnedAmbiguousActors = bAmbiguous;
}

void UWhaleObservationComponent::UpdateObservation()
{
	const APlayerController* Controller = Cast<APlayerController>(GetOwner());
	UWorld* World = GetWorld();
	if (!Controller || !World)
	{
		return;
	}
	const double Now = World->GetTimeSeconds();
	if (Now >= NextActorSearchTime)
	{
		ResolveSceneActors();
		NextActorSearchTime = Now + 2.0;
	}

	const AMqttVehiclePawn* Pawn = Cast<AMqttVehiclePawn>(Controller->GetPawn());
	const bool bTargetsAvailable = Controller->IsLocalPlayerController() && IsValid(Pawn)
		&& ObservationAnchor.IsValid() && WhaleSubject.IsValid()
		&& ObservationAnchor->ActorHasTag(ObservationAnchorTag) && WhaleSubject->ActorHasTag(WhaleSubjectTag);
	const FVector Offset = bTargetsAvailable
		? Pawn->GetActorLocation() - ObservationAnchor->GetActorLocation() : FVector::ZeroVector;
	const double HorizontalDistanceCm = FMath::Sqrt(Offset.X * Offset.X + Offset.Y * Offset.Y);
	const bool bWasInside = Progress.IsInside();
	// Swimming explorers may observe above the pad; the pad still defines horizontal discovery.
	const double ObservationHeight = Pawn && Pawn->IsUnderwaterMovementEnabled()
		? FMath::Max(HeightToleranceCm, Pawn->MaximumRiseAboveStart + 300.0f) : HeightToleranceCm;
	const bool bDiscoveredNow = Progress.Update(bTargetsAvailable, HorizontalDistanceCm, Offset.Z,
		EnterRadiusCm, ExitRadiusCm, ObservationHeight);
	if (bDiscoveredNow)
	{
		DiscoveryNoticeUntil = Now + 5.0;
	}
	if (!Progress.IsInside())
	{
		DiscoveryNoticeUntil = 0.0;
	}
	State.bAvailable = Progress.IsAvailable();
	State.bInObservationZone = Progress.IsInside();
	State.bWhaleDiscovered = Progress.HasDiscoveredWhale();
	State.bShowDiscoveryNotice = State.bInObservationZone && Now < DiscoveryNoticeUntil;
	State.DistanceToPadMeters = State.bAvailable ? static_cast<float>(HorizontalDistanceCm / 100.0) : 0.0f;
	OnObservationStateChanged.Broadcast(State);
	if (bObservationActive && bWasInside != State.bInObservationZone)
	{
		OnObservationZoneChanged.Broadcast(State.bInObservationZone);
	}
	if (bObservationActive && bDiscoveredNow && WhaleSubject.IsValid())
	{
		OnWhaleDiscovered.Broadcast(WhaleSubject.Get());
	}
}
