#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubmersiblePropulsionComponent.generated.h"
class AMqttVehiclePawn;
class UStaticMesh;
class UMaterialInterface;
class UInstancedStaticMeshComponent;
class USceneComponent;
/** Cosmetic propulsion only: measured Unreal motion, no input or MQTT commands. */
UCLASS(ClassGroup=(Exploration))
class MQTTVEHICLE_API USubmersiblePropulsionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 USubmersiblePropulsionComponent();
 int32 GetLiveBubbleCount() const;
 float GetRotorAngle() const { return RotorAngle; }
 float GetActivity() const { return Activity; }
protected:
 virtual void BeginPlay() override;
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
 friend class FPropulsionEffectsTest;
 void UpdateEffects(float DeltaTime);
 void EmitBubble(int32 Side);
 UPROPERTY() TObjectPtr<UStaticMesh> BladeMesh;
 UPROPERTY() TObjectPtr<UStaticMesh> BubbleMesh;
 UPROPERTY() TObjectPtr<UMaterialInterface> BubbleMaterial;
 UPROPERTY() TObjectPtr<UMaterialInterface> BladeMaterial;
 UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> Bubbles;
 UPROPERTY(Transient) TArray<TObjectPtr<USceneComponent>> Parts;
 UPROPERTY(Transient) TArray<TObjectPtr<USceneComponent>> Rotors;
 TWeakObjectPtr<AMqttVehiclePawn> Vehicle;
 struct FBubble { FVector Position=FVector::ZeroVector, Velocity=FVector::ZeroVector; float Age=2, Radius=2; };
 TArray<FBubble> Pool;
 FRandomStream Random{419};
 FVector PreviousPosition=FVector::ZeroVector;
 float PreviousYaw=0, Activity=0, RotorAngle=0, SpawnAccumulator=0;
 int32 NextBubble=0;
};
