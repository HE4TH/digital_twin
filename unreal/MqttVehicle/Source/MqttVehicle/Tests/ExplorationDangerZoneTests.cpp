#if WITH_DEV_AUTOMATION_TESTS

#include "ExplorationDangerZone.h"
#include "Misc/AutomationTest.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationDangerZoneTest, "MqttVehicle.Exploration.DangerZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExplorationDangerZoneTest::RunTest(const FString& Parameters)
{
	FExplorationDangerZoneState Zone;
	const FVector Extent(100, 200, 300);
	TestFalse(TEXT("Outside"), Zone.Update(FVector(250, 0, 0), Extent));
	TestTrue(TEXT("Boundary entry"), Zone.Update(FVector(100, 0, 0), Extent));
	TestFalse(TEXT("No restart while inside after a user cancellation"), Zone.Update(FVector::ZeroVector, Extent));
	Zone.Update(FVector(190, 0, 0), Extent);
	TestTrue(TEXT("Exit margin retains occupancy"), Zone.bInside);
	TestFalse(TEXT("Boundary jitter does not trigger again"), Zone.Update(FVector(90, 0, 0), Extent));
	Zone.Update(FVector(201, 0, 0), Extent);
	TestFalse(TEXT("Leave outer margin"), Zone.bInside);
	TestTrue(TEXT("Re-enter after a real exit"), Zone.Update(FVector::ZeroVector, Extent));
	Zone.Update(FVector(std::numeric_limits<double>::infinity(), 0, 0), Extent);
	TestFalse(TEXT("Invalid sample does not re-arm"), Zone.Update(FVector::ZeroVector, Extent));
	Zone.Update(FVector(0, 0, 401), Extent);
	TestFalse(TEXT("Vertical exit"), Zone.bInside);
	TestFalse(TEXT("Invalid extent is ignored"), Zone.Update(FVector::ZeroVector, FVector(-1, 1, 1)));

	FExplorationDangerZoneState Crossing;
	Crossing.Update(FVector(-150, 0, 0), Extent);
	TestTrue(TEXT("Fast continuous pass through a narrow box"), Crossing.Update(FVector(150, 0, 0), Extent));
	TestFalse(TEXT("No repeated pass event"), Crossing.Update(FVector(160, 0, 0), Extent));
	FExplorationDangerZoneState Jump;
	Jump.Update(FVector(-1000, 0, 0), Extent);
	TestFalse(TEXT("Teleport across zone is not a traversal"), Jump.Update(FVector(1000, 0, 0), Extent));
	TestTrue(TEXT("Teleport landing inside still warns"), Jump.Update(FVector::ZeroVector, Extent));
	FExplorationDangerZoneState Above;
	Above.Update(FVector(-150, 0, 301), Extent);
	TestFalse(TEXT("Pass above the volume"), Above.Update(FVector(150, 0, 301), Extent));
	return true;
}

#endif
