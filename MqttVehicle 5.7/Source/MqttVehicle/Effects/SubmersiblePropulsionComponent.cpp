#include "SubmersiblePropulsionComponent.h"
#include "MqttVehiclePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

USubmersiblePropulsionComponent::USubmersiblePropulsionComponent()
{
 PrimaryComponentTick.bCanEverTick=true; PrimaryComponentTick.TickGroup=TG_PostPhysics;
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Foam(TEXT("/Game/Underwater/Materials/M_ThrusterBubble.M_ThrusterBubble"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Metal(TEXT("/Game/Underwater/Materials/M_Station.M_Station"));
 BladeMesh=Cube.Object; BubbleMesh=Sphere.Object; BubbleMaterial=Foam.Object; BladeMaterial=Metal.Object;
}
void USubmersiblePropulsionComponent::BeginPlay()
{
 Super::BeginPlay(); Vehicle=Cast<AMqttVehiclePawn>(GetOwner());
 if(!Vehicle.IsValid() || !Vehicle->HasSubmersibleChoices()) { SetComponentTickEnabled(false); return; }
 PreviousPosition=Vehicle->GetActorLocation(); PreviousYaw=Vehicle->GetActorRotation().Yaw;
 // Measured rear openings of the original Lanzaman thruster housings, in hull centimetres.
 for(int32 Side=0;Side<2;++Side)
 {
  auto* Root=NewObject<USceneComponent>(GetOwner(),*FString::Printf(TEXT("ThrusterRotor%d"),Side));
  Root->SetupAttachment(Vehicle->SubmersibleHull); Root->SetRelativeLocation(FVector(-54,Side==0 ? -183.1 : 183.1,Side==0 ? 43.3 : 42.2));
  Root->RegisterComponent(); Parts.Add(Root); Rotors.Add(Root);
  for(int32 Blade=0;Blade<3;++Blade)
  {
   const float Angle=Blade*120.0f;
   auto* Part=NewObject<UStaticMeshComponent>(GetOwner()); Part->SetupAttachment(Root);
   Part->SetStaticMesh(BladeMesh); Part->SetMaterial(0,BladeMaterial);
   Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Part->SetGenerateOverlapEvents(false); Part->SetCanEverAffectNavigation(false);
   Part->SetRelativeLocation(FRotator(0,0,Angle).RotateVector(FVector(0,11,0)));
   Part->SetRelativeRotation(FRotator(0,0,Angle+18)); Part->SetRelativeScale3D(FVector(.035,.27,.075));
   Part->RegisterComponent(); Parts.Add(Part);
  }
 }
 Bubbles=NewObject<UInstancedStaticMeshComponent>(GetOwner(),TEXT("ThrusterBubbles"));
 Bubbles->SetupAttachment(Vehicle->GetRootComponent()); Bubbles->SetAbsolute(true,true,true);
 Bubbles->SetWorldTransform(FTransform::Identity); Bubbles->SetStaticMesh(BubbleMesh); Bubbles->SetMaterial(0,BubbleMaterial);
 Bubbles->SetCollisionEnabled(ECollisionEnabled::NoCollision); Bubbles->SetGenerateOverlapEvents(false); Bubbles->SetCanEverAffectNavigation(false); Bubbles->SetCastShadow(false);
 Bubbles->RegisterComponent(); Parts.Add(Bubbles); Pool.SetNum(128);
 for(int32 I=0;I<Pool.Num();++I) Bubbles->AddInstance(FTransform(FQuat::Identity,FVector::ZeroVector,FVector::ZeroVector));
}
int32 USubmersiblePropulsionComponent::GetLiveBubbleCount() const
{
 int32 Count=0; for(const auto& Bubble:Pool) if(Bubble.Age<1.6f) ++Count; return Count;
}
void USubmersiblePropulsionComponent::EmitBubble(int32 Side)
{
 FBubble& Bubble=Pool[NextBubble]; NextBubble=(NextBubble+1)%Pool.Num();
 const FTransform Hull=Vehicle->SubmersibleHull->GetComponentTransform();
 const FVector Outlet=Rotors[Side]->GetRelativeLocation()+FVector(-7,Random.FRandRange(-13,13),Random.FRandRange(-13,13));
 Bubble.Position=Hull.TransformPosition(Outlet);
 Bubble.Velocity=Hull.TransformVectorNoScale(FVector(-Random.FRandRange(55,95),Random.FRandRange(-12,12),0))+FVector(0,0,Random.FRandRange(25,45));
 Bubble.Age=0; Bubble.Radius=Random.FRandRange(2.5,5.0);
}
void USubmersiblePropulsionComponent::UpdateEffects(float DeltaTime)
{
 if(!Vehicle.IsValid() || !Bubbles || DeltaTime<=0 || !FMath::IsFinite(DeltaTime)) return;
 const FVector Position=Vehicle->GetActorLocation(); const float Yaw=Vehicle->GetActorRotation().Yaw;
 const float Distance=FVector::Dist(Position,PreviousPosition);
 const float Turn=FMath::Abs(FMath::FindDeltaAngleDegrees(PreviousYaw,Yaw))/DeltaTime;
 const float Speed=Distance/DeltaTime;
 const bool bTeleport=Distance>500 || !FMath::IsFinite(Speed);
 const float Target=bTeleport ? 0 : FMath::Clamp(FMath::Max(Speed/268.224f,Turn/90.0f),0.0f,1.0f);
 PreviousPosition=Position; PreviousYaw=Yaw;
 Activity=FMath::FInterpTo(Activity,Target,FMath::Min(DeltaTime,.1f),8);
 if(Target==0 && Activity<.01f) Activity=0;
 RotorAngle=FMath::Fmod(RotorAngle+Activity*1080*DeltaTime,360.0f);
 for(int32 I=0;I<Rotors.Num();++I) Rotors[I]->SetRelativeRotation(FRotator(0,0,(I==0 ? 1 : -1)*RotorAngle));
 if(bTeleport) { for(auto& Bubble:Pool) Bubble.Age=2; SpawnAccumulator=0; }
 SpawnAccumulator+=Activity*36*FMath::Min(DeltaTime,.1f);
 while(SpawnAccumulator>=1) { EmitBubble(0); EmitBubble(1); SpawnAccumulator-=1; }
 for(int32 I=0;I<Pool.Num();++I)
 {
  auto& Bubble=Pool[I]; Bubble.Age+=DeltaTime;
  FVector Scale=FVector::ZeroVector;
  if(Bubble.Age<1.6f)
  {
   Bubble.Position+=Bubble.Velocity*DeltaTime; Bubble.Velocity.Z+=18*DeltaTime;
   const float Fade=FMath::Clamp((1.6f-Bubble.Age)/.35f,0.0f,1.0f);
   Scale=FVector(Bubble.Radius/50*(1+Bubble.Age*.4f)*Fade);
  }
  Bubbles->UpdateInstanceTransform(I,FTransform(FQuat::Identity,Bubble.Position,Scale),true,false,true);
 }
 Bubbles->MarkRenderStateDirty();
}
void USubmersiblePropulsionComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction)
{ Super::TickComponent(DeltaTime,TickType,TickFunction); UpdateEffects(DeltaTime); }
void USubmersiblePropulsionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
 for(int32 I=Parts.Num()-1;I>=0;--I) if(Parts[I]) Parts[I]->DestroyComponent();
 Parts.Reset(); Rotors.Reset(); Bubbles=nullptr; Pool.Reset(); Super::EndPlay(Reason);
}
