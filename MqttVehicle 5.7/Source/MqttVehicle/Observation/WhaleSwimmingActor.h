#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WhaleSwimmingActor.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

/** Observation subject: authored swimming animation plus a small, continuous orbit. */
UCLASS()
class MQTTVEHICLE_API AWhaleSwimmingActor : public AActor
{
	GENERATED_BODY()
public:
	AWhaleSwimmingActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Whale")
	TObjectPtr<USkeletalMeshComponent> WhaleMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Whale")
	TObjectPtr<UAnimSequence> SwimmingAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Whale", meta=(ClampMin="100", Units="cm"))
	float OrbitRadius = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Whale", meta=(ClampMin="10", Units="s"))
	float OrbitPeriod = 55.0f;
	virtual void Tick(float DeltaSeconds) override;
protected:
	virtual void BeginPlay() override;
private:
	FVector OrbitCenter = FVector::ZeroVector;
	double Elapsed = 0.0;
};
