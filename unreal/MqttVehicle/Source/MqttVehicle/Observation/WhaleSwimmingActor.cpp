#include "WhaleSwimmingActor.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"

AWhaleSwimmingActor::AWhaleSwimmingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	WhaleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WhaleMesh"));
	SetRootComponent(WhaleMesh);
	WhaleMesh->SetCollisionProfileName(TEXT("NoCollision"));
	WhaleMesh->SetGenerateOverlapEvents(false);
	WhaleMesh->SetCanEverAffectNavigation(false);
	Tags.Add(TEXT("WhaleObservationSubject"));
}

void AWhaleSwimmingActor::BeginPlay()
{
	Super::BeginPlay();
	OrbitRadius = FMath::IsFinite(OrbitRadius) ? FMath::Max(100.0f, OrbitRadius) : 800.0f;
	OrbitPeriod = FMath::IsFinite(OrbitPeriod) ? FMath::Max(10.0f, OrbitPeriod) : 55.0f;
	// The saved actor location is the first point on the circle: no startup teleport.
	OrbitCenter = GetActorLocation() - FVector(OrbitRadius, 0.0, 0.0);
	if (SwimmingAnimation) WhaleMesh->PlayAnimation(SwimmingAnimation, true);
}

void AWhaleSwimmingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0f) return;
	Elapsed = FMath::Fmod(Elapsed + DeltaSeconds, static_cast<double>(OrbitPeriod));
	// The imported humpback faces local -X (Head_End is behind Tail_J5 on X).
	// Clockwise travel with this heading keeps its nose aligned with the tangent.
	const double Angle = -Elapsed * 2.0 * PI / OrbitPeriod;
	SetActorLocationAndRotation(OrbitCenter + FVector(OrbitRadius * FMath::Cos(Angle),
		OrbitRadius * FMath::Sin(Angle), 20.0 * FMath::Sin(2.0 * Angle)),
		FRotator(0.0, FMath::RadiansToDegrees(Angle) + 90.0, 0.0));
}
