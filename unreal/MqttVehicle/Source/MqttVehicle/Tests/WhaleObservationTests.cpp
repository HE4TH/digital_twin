#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "WhaleObservationComponent.h"
#include "WhaleObservationWidget.h"
#include "WhaleSwimmingActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWhaleObservationProgressTest,
	"MqttVehicle.Observation.ZoneAndDiscovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWhaleObservationProgressTest::RunTest(const FString& Parameters)
{
	// Pure state tests: no world, vehicle, MQTT connection or GPIO is created.
	FWhaleObservationProgress Progress;
	auto Update = [&Progress](double Distance, double Height = 0.0, bool bAvailable = true)
	{
		return Progress.Update(bAvailable, Distance, Height, 600.0, 750.0, 300.0);
	};
	TestFalse(TEXT("Approach does not discover"), Update(601.0));
	TestFalse(TEXT("Approach remains outside"), Progress.IsInside());
	TestFalse(TEXT("Passing above pad does not discover"), Update(0.0, 301.0));
	TestTrue(TEXT("Entry boundary discovers once"), Update(600.0, 300.0));
	TestTrue(TEXT("Discovery is retained"), Progress.HasDiscoveredWhale());
	TestFalse(TEXT("Inside sample does not rediscover"), Update(590.0));
	TestFalse(TEXT("Exit boundary does not rediscover"), Update(750.0));
	TestTrue(TEXT("Exit radius prevents boundary chatter"), Progress.IsInside());
	Update(751.0);
	TestFalse(TEXT("Beyond exit radius leaves zone"), Progress.IsInside());
	Update(650.0);
	TestFalse(TEXT("Reentry must reach entry radius"), Progress.IsInside());
	TestFalse(TEXT("Reentry does not rediscover"), Update(500.0));
	TestTrue(TEXT("Reentry still allows observation"), Progress.IsInside());
	Update(0.0, -301.0);
	TestFalse(TEXT("Passing below pad exits"), Progress.IsInside());
	Update(500.0);
	Update(0.0, 0.0, false);
	TestFalse(TEXT("Missing pawn or target disables UI"), Progress.IsAvailable());
	TestFalse(TEXT("Missing target exits zone"), Progress.IsInside());
	TestTrue(TEXT("Loss/respawn preserves session discovery"), Progress.HasDiscoveredWhale());
	TestFalse(TEXT("Restoring targets does not rediscover"), Update(0.0));
	Update(std::numeric_limits<double>::quiet_NaN());
	TestFalse(TEXT("Nonfinite distance is unavailable"), Progress.IsAvailable());
	Update(0.0, std::numeric_limits<double>::infinity());
	TestFalse(TEXT("Nonfinite height is unavailable"), Progress.IsAvailable());
	Progress.Update(true, 0.0, 0.0, 600.0, 500.0, 300.0);
	TestFalse(TEXT("Reversed radius configuration is unavailable"), Progress.IsAvailable());

	FWhaleObservationProgress NewSession;
	TestFalse(TEXT("New session clears discovery"), NewSession.HasDiscoveredWhale());
	TestFalse(TEXT("Absent targets never discover"), NewSession.Update(false, 0.0, 0.0, 600.0, 750.0, 300.0));
	TestTrue(TEXT("New session may discover again"), NewSession.Update(true, 0.0, 0.0, 600.0, 750.0, 300.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWhaleSwimmingTest, "MqttVehicle.Observation.SwimmingAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWhaleSwimmingTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Fixture;
	if (!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
	auto* Mesh = LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Underwater/Selected/Whale/SK_Humpback.SK_Humpback"));
	auto* Anim = LoadObject<UAnimSequence>(nullptr,TEXT("/Game/Underwater/Selected/Whale/SK_Humpback_Anim.SK_Humpback_Anim"));
	if (!TestNotNull(TEXT("Saved whale mesh"),Mesh) || !TestNotNull(TEXT("Saved swimming animation"),Anim)) return false;
	TestNotNull(TEXT("Whale material survives save and reload"), Mesh->GetMaterials()[0].MaterialInterface.Get());
	TestEqual(TEXT("Animation uses the imported whale skeleton"), Mesh->GetSkeleton(), Anim->GetSkeleton());
	TestTrue(TEXT("Animation contains a swimming cycle"), Anim->GetPlayLength() > 1.0f);
	const FVector Start(4900,2500,1300);
	auto* Whale = Fixture.GetTestWorld()->SpawnActor<AWhaleSwimmingActor>(Start,FRotator(0,90,0));
	Whale->WhaleMesh->SetSkeletalMesh(Mesh);
	Whale->SwimmingAnimation=Anim;
	Fixture.BeginPlayInTestWorld();
	Whale->DispatchBeginPlay();
	TestTrue(TEXT("No startup teleport"), Whale->GetActorLocation().Equals(Start,.01));
	TestTrue(TEXT("Imported nose and tail bones exist"), Whale->WhaleMesh->GetBoneIndex(TEXT("Head_End")) != INDEX_NONE
		&& Whale->WhaleMesh->GetBoneIndex(TEXT("Tail_J5")) != INDEX_NONE);
	for (int32 Step = 0; Step < 24; ++Step)
	{
		const FVector Before = Whale->GetActorLocation();
		Whale->Tick(Whale->OrbitPeriod / 24.0f);
		const FVector Travel = (Whale->GetActorLocation() - Before).GetSafeNormal2D();
		const FVector Nose = (Whale->WhaleMesh->GetSocketLocation(TEXT("Head_End"))
			- Whale->WhaleMesh->GetSocketLocation(TEXT("Tail_J5"))).GetSafeNormal2D();
		TestTrue(TEXT("Imported whale travels nose first throughout the circle"), FVector::DotProduct(Travel, Nose) > 0.95);
	}
	Whale->Tick(Whale->OrbitPeriod/4.0f);
	TestTrue(TEXT("Whale moves around the observation area"), FVector::Dist2D(Whale->GetActorLocation(),Start)>500.0);
	TestTrue(TEXT("Orbit reverses to clockwise"), Whale->GetActorLocation().Y < Start.Y - 700.0);
	Whale->Tick(Whale->OrbitPeriod*3.0f/4.0f);
	TestTrue(TEXT("Full cycle closes without drift"), Whale->GetActorLocation().Equals(Start,.01));
	Fixture.ForwardErrorMessages(this);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEncounterNoticeTest,"MqttVehicle.Observation.EncounterNotices",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FEncounterNoticeTest::RunTest(const FString& Parameters)
{
 auto* Widget=NewObject<UWhaleObservationWidget>();
 FWhaleObservationSnapshot Whale; Whale.bAvailable=true; Whale.bInObservationZone=true;
 Widget->ApplyObservationState(Whale); TestEqual(TEXT("Whale entry shows five-second notice"),Widget->WhaleSeconds,5.0f);
 Widget->WhaleSeconds=2; Widget->ApplyObservationState(Whale);
 TestEqual(TEXT("Repeated inside updates do not restart notice"),Widget->WhaleSeconds,2.0f);
 FExplorationReturnSnapshot Danger; Danger.Mode=EExplorationReturnMode::Manual; Danger.DangerEncounterSerial=1;
 Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Quick danger crossing still produces notice"),Widget->JellySeconds,5.0f);
 TestEqual(TEXT("Jelly warning does not overwrite whale notice"),Widget->WhaleSeconds,2.0f);
 Widget->JellySeconds=1; Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Same danger event does not restart notice"),Widget->JellySeconds,1.0f);
 Whale.bInObservationZone=false; Widget->ApplyObservationState(Whale);
 Whale.bInObservationZone=true; Widget->ApplyObservationState(Whale);
 TestEqual(TEXT("Reentry shows whale notice again"),Widget->WhaleSeconds,5.0f);
 ++Danger.DangerEncounterSerial; Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("New danger encounter restarts notice"),Widget->JellySeconds,5.0f);
 Danger.Mode=EExplorationReturnMode::Returning;Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Return entry shows three second toast"),Widget->ReturnSeconds,3.0f);
 Widget->ReturnSeconds=1;Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Repeated return state does not extend toast"),Widget->ReturnSeconds,1.0f);
 Danger.Mode=EExplorationReturnMode::Arrived;Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Arrival replaces toast"),Widget->ReturnSeconds,3.0f);
 Danger.Mode=EExplorationReturnMode::Manual;Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Ordinary manual state clears toast"),Widget->ReturnSeconds,0.0f);
 Danger.Mode=EExplorationReturnMode::Returning;Widget->ApplyReturnState(Danger);
 Danger.Mode=EExplorationReturnMode::Manual;Widget->ApplyReturnState(Danger);
 TestEqual(TEXT("Manual interruption is announced"),Widget->ReturnSeconds,3.0f);

 return !HasAnyErrors();
}

#endif
