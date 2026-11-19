#if WITH_DEV_AUTOMATION_TESTS

#include "ExplorationRoute.h"
#include "UnderwaterPathPlanner.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ExplorationReturnComponent.h"
#include "MqttVehiclePawn.h"
#include "MqttVehicle.h"
#include "MqttVehicleBridgeSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationRouteTest, "MqttVehicle.Exploration.RoutePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExplorationRouteTest::RunTest(const FString& Parameters)
{
	FExplorationRoute Route;
	const FVector Forward = FVector::ForwardVector;
	FExplorationRoute VerticalRoute;
	VerticalRoute.Begin(FVector::ZeroVector, Forward, FVector::ZeroVector);
	VerticalRoute.Record(FVector(0,0,100),Forward);
	VerticalRoute.Record(FVector(0,0,200),Forward);
	TestTrue(TEXT("Pure ascent contributes to remaining route length"), FMath::IsNearlyEqual(VerticalRoute.RemainingDistance(1,FVector(0,0,200)),200.0f));
	TestTrue(TEXT("3D distance distinguishes stacked crossings"), FMath::IsNearlyEqual(ExplorationRoute::DistanceToSegment3D(FVector(50,0,500),FVector::ZeroVector,FVector(100,0,0)),500.0));
	TestFalse(TEXT("Manual trim rejects another depth at the same XY"), VerticalRoute.TrimForManual(1,FVector(0,0,1000),Forward));
	for (float Side : {1.0f, -1.0f})
	{
		const FVector Start(123, -456, 78);
		const FVector Heading = FRotator(0, 37, 0).Vector();
		const auto Turn = ExplorationRoute::PivotTurn(Start, Heading, Side * 180.0f);
		TestTrue(TEXT("Stationary turn preserves the starting point"), !Turn.IsEmpty() && Turn.Last().Position.Equals(Start, 0.01));
		TestTrue(TEXT("Stationary turn ends facing home"), !Turn.IsEmpty() && FVector::DotProduct(Turn.Last().Forward, Heading) < -0.999);
		for (int32 I=1; I<Turn.Num(); ++I)
		{
			TestTrue(TEXT("Every turn sample stays in place"), Turn[I].Position.Equals(Start));
			TestTrue(TEXT("Turn never changes elevation"), FMath::IsNearlyEqual(Turn[I].Position.Z, Start.Z));
		}
	}
	TestTrue(TEXT("Reject invalid turn angle"), ExplorationRoute::PivotTurn(FVector::ZeroVector,Forward,std::numeric_limits<float>::quiet_NaN()).IsEmpty());
	TestFalse(TEXT("Cannot invent unrecorded route to home"), Route.Begin(FVector(1000, 0, 0), Forward, FVector::ZeroVector));
	TestTrue(TEXT("Start near home"), Route.Begin(FVector::ZeroVector, Forward, FVector::ZeroVector));
	Route.Record(FVector(10, 0, 0), Forward);
	TestEqual(TEXT("Small drift does not fill history"), Route.Points.Num(), 1);
	Route.Record(FVector(100, 0, 0), Forward);
	Route.Record(FVector(200, 0, 0), Forward);
	TestEqual(TEXT("Record actual movement"), Route.Points.Num(), 3);
	TestEqual(TEXT("Outbound forward segment returns in reverse"), ExplorationRoute::ReturnDirection(Route.Points[2], Route.Points[1]), -1.0f);
	TestTrue(TEXT("Remaining path distance"), FMath::IsNearlyEqual(Route.RemainingDistance(1, FVector(200, 0, 0)), 200.0f));
	TestTrue(TEXT("Manual takeover trims consumed return path"), Route.TrimForManual(1, FVector(150, 0, 0), Forward));
	TestTrue(TEXT("Resume recording from takeover"), Route.Record(FVector(150, 100, 0), Forward, true));
	TestFalse(TEXT("Teleport invalidates, never connects across gap"), Route.Record(FVector(2000, 0, 0), Forward));
	TestFalse(TEXT("Invalid route cannot silently restart away from home"), Route.bValid);
	Route.Begin(FVector::ZeroVector, Forward, FVector::ZeroVector);
	TestFalse(TEXT("Nonfinite pose invalidates"), Route.Record(FVector(std::numeric_limits<double>::quiet_NaN(), 0, 0), Forward));
	Route.Begin(FVector::ZeroVector, Forward, FVector::ZeroVector);
	Route.Points.SetNum(4000);
	TestFalse(TEXT("Capacity does not discard home history"), Route.Record(FVector(100, 0, 0), Forward));

	const auto Reverse = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(-300, 100, 0), -1, 0, 200);
	TestTrue(TEXT("Reverse command"), Reverse.Throttle < 0.0f);
	TestTrue(TEXT("Reverse right target needs positive steering curvature"), Reverse.Steering > 0.0f);
	const auto ForwardLeft = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(300, -100, 0), 1, 0, 200);
	TestTrue(TEXT("Forward left command"), ForwardLeft.Throttle > 0.0f && ForwardLeft.Steering < 0.0f);
	const auto TurningLaunch = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(150, 80, 0), 1, 0, 6.0f / 0.036f);
	TestTrue(TEXT("Steered launch supplies torque without braking"), TurningLaunch.Throttle >= 0.5f && TurningLaunch.Brake == 0.0f);
	const auto Cruising = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(300, 0, 0), 1, 160, 6.0f / 0.036f);
	TestTrue(TEXT("Launch boost eases off near cruising speed"), Cruising.Throttle > 0.0f && Cruising.Throttle < TurningLaunch.Throttle);
	const auto WrongGear = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(-300, 0, 0), -1, 100, 200);
	const auto CorrectingAtThree = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(300,60,0), 1, 3*44.704f, 5*44.704f);
	TestTrue(TEXT("Moderate steering at three MPH still accelerates toward five"), CorrectingAtThree.Throttle > 0.5f && CorrectingAtThree.Brake == 0.0f);
	const auto CorrectingAtFive = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(300,60,0), 1, 5*44.704f, 5*44.704f);
	TestTrue(TEXT("Gentle correction maintains five MPH without braking"), CorrectingAtFive.Throttle > 0.0f && CorrectingAtFive.Brake == 0.0f);
	const auto Arriving = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(150,0,0), 1, 5*44.704f, 90);
	TestTrue(TEXT("Arrival slowdown still applies"), Arriving.Throttle == 0.0f && Arriving.Brake > 0.0f);
	TestTrue(TEXT("Brake before changing travel direction"), WrongGear.Throttle == 0.0f && WrongGear.Brake == 1.0f);
	const auto Overspeed = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(300, 0, 0), 1, 400, 200);
	TestTrue(TEXT("Overspeed brakes rather than reversing"), Overspeed.Throttle == 0.0f && Overspeed.Brake > 0.0f);
	TestTrue(TEXT("Cross-track distance"), FMath::IsNearlyEqual(ExplorationRoute::DistanceToSegment2D(FVector(50, 100, 0), FVector::ZeroVector, FVector(100, 0, 0)), 100.0));
	FTestWorldWrapper Fixture;
	if (!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
	UWorld* World = Fixture.GetTestWorld();
	const auto Turn = ExplorationRoute::PivotTurn(FVector(0,0,18), Forward, 180.0f);
	TestTrue(TEXT("Empty turn space is clear"), ExplorationRoute::IsTurnClear(*World, Turn, nullptr));
	auto MakeBox = [World](const FVector& Location, const FVector& Extent)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Ignore);
		Box->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
		Box->RegisterComponent();
		Actor->SetActorLocation(Location);
		return Actor;
	};
	MakeBox(FVector(0,0,-25), FVector(5000,5000,25));
	TestTrue(TEXT("Floor below the wheels does not block turn planning"), ExplorationRoute::IsTurnClear(*World, Turn, nullptr));
	const FVector Sample = Turn[25].Position + FVector(300,0,0);
	AActor* Base = MakeBox(FVector(Sample.X,Sample.Y,25), FVector(32.5,32.5,25));
	TestFalse(TEXT("Low guide base blocks a turn even when invisible to visibility traces"), ExplorationRoute::IsTurnClear(*World, Turn, nullptr));
	Base->SetActorLocation(FVector(10000,10000,25));
	TestTrue(TEXT("Moving the obstacle away restores clearance"), ExplorationRoute::IsTurnClear(*World, Turn, nullptr));
	const auto SixMph = ExplorationRoute::Follow(FVector::ZeroVector, Forward, FVector(2000,0,0), 1, 255.0f, 6.0f * 44.704f);
	TestTrue(TEXT("Straight return is not capped at the former 250 cm/s"), SixMph.Brake == 0.0f && SixMph.Throttle > 0.18f);
	Fixture.ForwardErrorMessages(this);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationUnderwaterReturnTest, "MqttVehicle.Exploration.UnderwaterReturn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExplorationUnderwaterReturnTest::RunTest(const FString& Parameters)
{
	UClass* RoverClass = LoadClass<AMqttVehiclePawn>(nullptr,
		TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
	if (!TestNotNull(TEXT("Load saved underwater rover"), RoverClass)) return false;
	for (int32 Case = 0; Case < 16; ++Case)
	{
		const bool bObstacleCase = Case == 15;
		const int32 Scenario = bObstacleCase ? 0 : Case%5;
		const float Delta = 1.0f / (Case < 5 ? 60.0f : Case < 10 ? 30.0f : 20.0f);
		FTestWorldWrapper Fixture;
		if (!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
		UWorld* World = Fixture.GetTestWorld();
		auto* Rover = World->SpawnActor<AMqttVehiclePawn>(RoverClass, FVector(0,0,100), FRotator::ZeroRotator);
		auto* Owner = World->SpawnActor<AActor>();
		auto* Return = NewObject<UExplorationReturnComponent>(Owner);
		Return->RegisterComponent();
		auto* Home = World->SpawnActor<AActor>(); Home->SetActorLocation(Rover->GetActorLocation());
		Fixture.BeginPlayInTestWorld(); World->GetWorldSettings()->NotifyBeginPlay();
		// GamePreview never creates MQTT or a desktop viewport. Exercise the same
		// follower with real recorded manual movement; production focus guards stay intact.
		Return->SetComponentTickEnabled(false);
		Return->Vehicle = Rover; Return->HomeAnchor = Home; Return->HomePosition = Home->GetActorLocation();
		Return->Route.Begin(Rover->GetActorLocation(), Rover->GetActorForwardVector(), Return->HomePosition);
		auto Drive = [&](float Throttle, float Steering, float Up, float Down, int32 Frames)
		{
			Rover->DoThrottle(Throttle); Rover->DoSteering(Steering); Rover->DoAscend(Up); Rover->DoDescend(Down);
			for (int32 I=0; I<Frames; ++I)
			{
				Fixture.TickTestWorld(1.0f/60);
				if (I%6==0) Return->Route.Record(Rover->GetActorLocation(), Rover->GetActorForwardVector());
			}
			Return->Route.Record(Rover->GetActorLocation(), Rover->GetActorForwardVector(), true);
		};
		Drive(1,0,0,0,240);
		if (Scenario == 1) Drive(1,1,0,0,420);
		if (Scenario == 2) { Drive(1,0.65f,1,0,300); Drive(1,-0.8f,0,1,240); }
		if (Scenario == 3) { Drive(0,0,1,0,240); Drive(1,0,0,0,180); }
		if (Scenario == 4) { Drive(1,1,0,0,180); Drive(1,-1,0,0,180); }
		Drive(0,0,0,0,1);
		AActor* Obstacle = nullptr;
		if (bObstacleCase)
		{
			// A new obstacle blocks a formerly clear recorded segment. Never tunnel
			// through it or remove the watchdog merely to make the return succeed.
			Obstacle = World->SpawnActor<AActor>();
			auto* Wall = NewObject<UBoxComponent>(Obstacle);
			Obstacle->SetRootComponent(Wall); Wall->SetBoxExtent(FVector(20,1000,1000));
			Wall->SetCollisionProfileName(TEXT("BlockAll")); Wall->RegisterComponent();
			Obstacle->SetActorLocation(FVector(400,0,300));
			TestFalse(TEXT("Full hull lookahead rejects newly blocked route"), Rover->IsUnderwaterPathClear(FVector(0,0,100)));
		}
		Rover->BeginExplorationControl();
		Return->State.Mode = EExplorationReturnMode::Returning;
		Return->TargetIndex = Return->Route.Points.Num()-2;
		Return->BestRemainingDistance = Return->Route.RemainingDistance(Return->TargetIndex, Rover->GetActorLocation());
		const int32 Recorded = Return->Route.Points.Num();
		int32 Frames=0;
		float PeakSpeed=0;
		for (; Frames*Delta<120 && Return->State.Mode==EExplorationReturnMode::Returning; ++Frames)
		{
			Return->FollowRoute(Delta);
			Fixture.TickTestWorld(Delta);
			PeakSpeed=FMath::Max(PeakSpeed,static_cast<float>(Rover->GetVelocity().Size()/44.704));
		}
		const FString Result = FString::Printf(TEXT("Case %d at %.0f Hz: mode %d, points %d, cursor %d, %.2fs, peak %.3f MPH, position %s, detail %s"),
			Case, 1.0f/Delta, static_cast<int32>(Return->State.Mode), Recorded, Return->TargetIndex, Frames*Delta, PeakSpeed,
			*Rover->GetActorLocation().ToString(), *Return->State.Detail.ToString());
		AddInfo(Result);
		TestTrue(*Result, Return->State.Mode==(bObstacleCase ? EExplorationReturnMode::Stopped : EExplorationReturnMode::Arrived));
		TestTrue(TEXT("Automatic cruise stays within five MPH"), PeakSpeed<=5.01f);
		if (Scenario==0 && !bObstacleCase) TestTrue(TEXT("Straight return still reaches five MPH"), PeakSpeed>4.9f);
		if (bObstacleCase)
		{
			TestTrue(TEXT("Blocked return stops before crossing the wall"), Rover->GetActorLocation().X>420);
			TestFalse(TEXT("Blocked return releases automatic control"), Rover->IsExplorationControlActive());
			Obstacle->SetActorLocation(FVector(100000,0,0));
			Rover->BeginExplorationControl();
			Return->State.Mode=EExplorationReturnMode::Returning;
			Return->TargetIndex=Return->Route.Points.Num()-2;
			Return->NoProgressSeconds=Return->StoppedSeconds=0;
			Return->BestRemainingDistance=Return->Route.RemainingDistance(Return->TargetIndex,Rover->GetActorLocation());
			for (int32 I=0; I<2400 && Return->State.Mode==EExplorationReturnMode::Returning; ++I)
			{
				Return->FollowRoute(Delta); Fixture.TickTestWorld(Delta);
			}
			TestTrue(TEXT("Retry uses trimmed recorded route after obstacle is removed"), Return->State.Mode==EExplorationReturnMode::Arrived);
		}
		else TestTrue(TEXT("Return arrives within 25 cm in 3D"), FVector::Dist(Rover->GetActorLocation(),FVector(0,0,100))<=25.0);
		TestTrue(TEXT("Return stays isolated from RC commands"), Rover->GetVehicleControlInput().IsNearlyZero());
		Rover->StopVehicleInput(); Fixture.ForwardErrorMessages(this);
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnderwaterPlannerTest, "MqttVehicle.Exploration.PathPlanner3D",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUnderwaterPlannerTest::RunTest(const FString& Parameters)
{
 FUnderwaterPathPlanner Planner;
 FUnderwaterPathPlanner::FSettings Settings;
 Settings.Bounds=FBox(FVector(-300,-900,-150),FVector(1500,900,1200));
 const FVector Start(0,0,0), Goal(1200,0,0);
 auto Empty=[](const FVector&,const FVector&) { return true; };
 Planner.Begin(Start,Goal,Settings,Empty);
 TestTrue(TEXT("Clear space uses the globally shortest straight segment"),Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Succeeded && FMath::IsNearlyEqual(FUnderwaterPathPlanner::Length(Planner.GetPath()),1200.0));
 const FBox Wall(FVector(450,-2000,-1000),FVector(750,2000,450));
 auto Clear=[&](const FVector& A,const FVector& B) { return !FMath::LineBoxIntersection(Wall,A,B,B-A); };
 Planner.Begin(Start,Goal,Settings,Clear);
 int32 Slices=0;
 while(Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Searching || Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Smoothing)
 {
  Planner.Step(Clear,1,100); if(++Slices>10000) break;
 }
 TestTrue(TEXT("Incremental 3D search finds an overhead route"), Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Succeeded);
 TestTrue(TEXT("Work spans multiple bounded slices"),Slices>1);
 TestTrue(TEXT("Shortcut smoothing never lengthens the graph route"),FUnderwaterPathPlanner::Length(Planner.GetPath())<=Planner.GetGraphDistance()+0.01);
 double Highest=0;
 for(int32 I=0; I<Planner.GetPath().Num(); ++I)
 {
  Highest=FMath::Max(Highest,Planner.GetPath()[I].Z);
  TestTrue(TEXT("Every path point stays in the search volume"),Settings.Bounds.IsInsideOrOn(Planner.GetPath()[I]));
  if(I>0) TestTrue(TEXT("Every smoothed edge clears the wall"),Clear(Planner.GetPath()[I-1],Planner.GetPath()[I]));
 }
 TestTrue(TEXT("The route changes depth to pass above the wall"),Highest>450);
 // A narrow 2D lattice with a known shortest diagonal detour around one blocked cell.
 Settings.CellSize=150; Settings.Bounds=FBox(FVector(0,-150,0),FVector(600,150,0));
 auto CellBlocked=[](const FVector& A,const FVector& B)
 { const FBox Block(FVector(290,-10,-10),FVector(310,10,10)); return !FMath::LineBoxIntersection(Block,A,B,B-A); };
 Planner.Begin(Start,FVector(600,0,0),Settings,CellBlocked);
 for(int32 I=0; I<100; ++I) Planner.Step(CellBlocked,100,100);
 TestTrue(TEXT("A* graph cost matches the exact shortest 26-neighbour detour"),FMath::IsNearlyEqual(Planner.GetGraphDistance(),300.0+300.0*FMath::Sqrt(2.0),0.01));
 Settings.Bounds=FBox(FVector(-300,-900,-150),FVector(1500,900,300));
 Planner.Begin(Start,Goal,Settings,Clear);
 for(int32 I=0; I<1000 && Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Searching; ++I) Planner.Step(Clear,100,100);
 TestTrue(TEXT("A sealed volume reports failure without unsafe fallback"),Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Failed);
 Settings.Bounds.Max.Z=1200; Settings.MaxNodes=2;
 Planner.Begin(Start,Goal,Settings,Clear); Planner.Step(Clear,100,100);
 TestTrue(TEXT("Node capacity is enforced"),Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Failed);
 Planner.Begin(FVector(std::numeric_limits<double>::quiet_NaN(),0,0),Goal,Settings,Empty);
 TestTrue(TEXT("Nonfinite endpoints are rejected"),Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Failed);
 Planner.Reset(); TestTrue(TEXT("Cancellation releases a partial search"),Planner.GetStatus()==FUnderwaterPathPlanner::EStatus::Idle && Planner.GetPath().IsEmpty());
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationPlannedReturnTest, "MqttVehicle.Exploration.PlannedReturn",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FExplorationPlannedReturnTest::RunTest(const FString& Parameters)
{
 UClass* RoverClass=LoadClass<AMqttVehiclePawn>(nullptr,TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
 if(!RoverClass) return false;
 for(int32 Case=0; Case<8; ++Case)
 {
  FTestWorldWrapper Fixture; if(!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
  UWorld* World=Fixture.GetTestWorld();
  FVector Home(0,0,500), Start(3000,0,500);
  if(Case==5)
  {
   bool Loaded=false;
   auto* Level=ULevelStreamingDynamic::LoadLevelInstance(World,TEXT("/Game/Underwater/Maps/L_UnderwaterExploration"),FVector::ZeroVector,FRotator::ZeroRotator,Loaded);
   if(!TestTrue(TEXT("Load actual underwater scene as a collision test level"),Loaded && Level)) return false;
   Level->SetShouldBeVisible(true); World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
   TArray<AActor*> Starts; UGameplayStatics::GetAllActorsOfClass(World,APlayerStart::StaticClass(),Starts);
   if(!TestTrue(TEXT("Saved scene contains the actual departure pose"),Starts.Num()>0)) return false;
   Home=Starts[0]->GetActorLocation(); Start=Home;
  }
  auto* Rover=World->SpawnActor<AMqttVehiclePawn>(RoverClass,Home,FRotator::ZeroRotator);
  auto* Owner=World->SpawnActor<AActor>(); auto* Return=NewObject<UExplorationReturnComponent>(Owner); Return->RegisterComponent();
  Fixture.BeginPlayInTestWorld(); World->GetWorldSettings()->NotifyBeginPlay();
  Return->SetComponentTickEnabled(false); Return->bActive=true;
  Return->Vehicle=Rover; Return->NavigationHome=Home; Return->bNavigationHomeValid=true;
  if(Case==5)
  {
   bool Found=false;
   for(const FVector Offset : {FVector(1800,0,500),FVector(3000,2000,600),FVector(4500,2000,800)})
   {
    if(Rover->IsUnderwaterNavigationSegmentClear(Home+Offset,Home+Offset)) { Start=Home+Offset; Found=true; break; }
   }
   if(!TestTrue(TEXT("Find an unrecorded clear pose in the actual scene"),Found)) return false;
  }
  Rover->SetActorLocation(Start,false,nullptr,ETeleportType::TeleportPhysics);
  auto MakeWall=[&](FVector Position,FVector Extent)
  {
   AActor* Actor=World->SpawnActor<AActor>(); auto* Box=NewObject<UBoxComponent>(Actor);
   Actor->SetRootComponent(Box); Box->SetBoxExtent(Extent); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent(); Actor->SetActorLocation(Position); return Actor;
  };
  if(Case==1) MakeWall(FVector(1500,0,500),FVector(80,600,2000));
  if(Case==3) MakeWall(Start,FVector(50,50,50));
  if(Case>=6)
  {
   auto* Danger=World->SpawnActor<ATriggerBox>(Case==6 ? FVector(1500,0,500) : Start,FRotator::ZeroRotator);
   auto* Box=Cast<UBoxComponent>(Danger->GetCollisionComponent()); Box->SetBoxExtent(FVector(350,350,1000));
   Box->SetCollisionEnabled(ECollisionEnabled::NoCollision); Danger->Tags.Add(TEXT("ExplorationDangerZone")); Return->RefreshDangerZones();
  }
  TestTrue(TEXT("Navigation does not require any recorded outbound poses"),Return->Route.Points.IsEmpty());
  Rover->BeginExplorationControl(); Return->BeginNavigationPlan();
  if(Case==4)
  {
   Return->ResumeManual(); TestFalse(TEXT("M cancels planning and releases control"),Rover->IsExplorationControlActive());
   TestTrue(TEXT("Cancelled planner is idle"),Return->Navigator.GetStatus()==FUnderwaterPathPlanner::EStatus::Idle); continue;
  }
  bool AddedObstacle=false; float Peak=0; int32 Frames=0; float Length=0;
  bool SawArrivalTurn=false; float ArrivalYaw=0; FVector ArrivalPosition=FVector::ZeroVector;
  const float Delta=Case%2 ? 1.0f/30 : 1.0f/60;
  for(; Frames*Delta<120 && Return->IsAutomatic(); ++Frames)
  {
   if(Return->State.Mode==EExplorationReturnMode::Planning) Return->TickNavigationPlan(Delta);
   else if(Return->State.Mode==EExplorationReturnMode::Returning)
   {
    if(Length==0) Length=Return->NavigationRemaining();
    if(Case==2 && !AddedObstacle && Rover->GetActorLocation().X<2600)
    { MakeWall(FVector(1500,0,500),FVector(80,600,2000)); AddedObstacle=true; }
    Return->FollowRoute(Delta);
    if(Return->bArrivalTurn && !SawArrivalTurn)
    { SawArrivalTurn=true; ArrivalYaw=Rover->GetActorRotation().Yaw; ArrivalPosition=Rover->GetActorLocation(); }
    if(SawArrivalTurn) TestTrue(TEXT("Arrival rotation has no translation"), Rover->GetActorLocation().Equals(ArrivalPosition,0.1));
   }
   Fixture.TickTestWorld(Delta); Peak=FMath::Max(Peak,static_cast<float>(Rover->GetVelocity().Size()/44.704));
  }
  const bool ExpectedStop=Case==3;
  const FString Label=FString::Printf(TEXT("Planned case %d: mode=%d duration=%.2f length=%.1f replans=%d pos=%s detail=%s"),Case,static_cast<int32>(Return->State.Mode),Frames*Delta,Length,Return->ReplanCount,*Rover->GetActorLocation().ToString(),*Return->State.Detail.ToString());
  AddInfo(Label); TestTrue(*Label,Return->State.Mode==(ExpectedStop ? EExplorationReturnMode::Stopped : EExplorationReturnMode::Arrived));
  if(!ExpectedStop) TestTrue(TEXT("Arrives within 25cm of the departure pose"),FVector::Dist(Rover->GetActorLocation(),Home)<=25);
  if(!ExpectedStop)
  {
   TestTrue(TEXT("Arrival includes a stationary half turn"),SawArrivalTurn);
   TestTrue(TEXT("Final heading is opposite the arrival heading"), FMath::Abs(FMath::Abs(FMath::FindDeltaAngleDegrees(ArrivalYaw,static_cast<float>(Rover->GetActorRotation().Yaw)))-180.0f)<0.2f);
  }
  if(Case==0) TestTrue(TEXT("Clear return is the direct 30m line rather than travel history"),FMath::IsNearlyEqual(Length,3000.0f,0.1f));
  if(Case==1) TestEqual(TEXT("A static obstacle does not need replanning at corners"),Return->ReplanCount,0);
  if(Case==2) TestTrue(TEXT("A new obstacle triggers a bounded replan"),Return->ReplanCount>=1 && Return->ReplanCount<=3);
  TestTrue(TEXT("Navigation respects 5 MPH and never commands the RC car"),Peak<=5.01 && Rover->GetVehicleControlInput().IsNearlyZero());
  TestFalse(TEXT("Completed or failed navigation releases control"),Rover->IsExplorationControlActive());
  Fixture.ForwardErrorMessages(this);
 }
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationSceneReturnTest, "MqttVehicle.Exploration.SceneReturn",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FExplorationSceneReturnTest::RunTest(const FString& Parameters)
{
 FTestWorldWrapper Fixture; if(!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
 UWorld* World=Fixture.GetTestWorld(); bool Loaded=false;
 auto* Level=ULevelStreamingDynamic::LoadLevelInstance(World,TEXT("/Game/Underwater/Maps/L_UnderwaterExploration"),FVector::ZeroVector,FRotator::ZeroRotator,Loaded);
 if(!TestTrue(TEXT("Load scene"),Loaded && Level)) return false;
 Level->SetShouldBeVisible(true); World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
 TArray<AActor*> Starts; UGameplayStatics::GetAllActorsOfClass(World,APlayerStart::StaticClass(),Starts);
 if(Starts.IsEmpty()) return false;
 const FVector Home=Starts[0]->GetActorLocation();
 UClass* RoverClass=LoadClass<AMqttVehiclePawn>(nullptr,TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
 auto* Rover=World->SpawnActor<AMqttVehiclePawn>(RoverClass,Home,Starts[0]->GetActorRotation());
 auto* Owner=World->SpawnActor<AActor>(); auto* Return=NewObject<UExplorationReturnComponent>(Owner); Return->RegisterComponent();
 Fixture.BeginPlayInTestWorld(); World->GetWorldSettings()->NotifyBeginPlay();
 Return->SetComponentTickEnabled(false); Return->bActive=true; Return->Vehicle=Rover;
 Return->NavigationHome=Home; Return->bNavigationHomeValid=true; Return->RefreshDangerZones();
 // Re-entry is re-armed only on exit; duplicate discovery/zone callbacks coalesce.
 Return->OnObservationZoneChanged(true);
 TestTrue(TEXT("First entry schedules a return"),Return->bDiscoveryPending);
 Return->bDiscoveryPending=false;
 Return->OnWhaleDiscovered(nullptr); Return->OnObservationZoneChanged(true);
 TestFalse(TEXT("Staying inside cannot repeatedly request a return"),Return->bDiscoveryPending);
 Return->StopWithReason(FText::FromString(TEXT("Simulated connection failure")));
 Return->OnObservationZoneChanged(false); Return->ResumeManual();
 Return->OnObservationZoneChanged(true);
 TestTrue(TEXT("Re-entry after failure and manual return can retry"),Return->bDiscoveryPending);
 Return->bDiscoveryPending=false; Return->StopExploration(); Return->OnObservationZoneChanged(true);
 TestFalse(TEXT("Explicit stop while inside is not undone"),Return->bDiscoveryPending);
 Return->OnObservationZoneChanged(false); Return->ResumeManual();
 Return->State.Mode=EExplorationReturnMode::Arrived; Return->OnObservationZoneChanged(true);
 TestTrue(TEXT("Another visit after arrival can retry"),Return->bDiscoveryPending);
 Return->bDiscoveryPending=false;
 AddInfo(FString::Printf(TEXT("Scene home=%s envelope=%s clear=%d"),*Home.ToString(),*Rover->GetUnderwaterNavigationEnvelope().ToString(),Rover->IsUnderwaterNavigationSegmentClear(Home,Home)));
 for(const FName Tag : {FName(TEXT("WhaleObservationAnchor")),FName(TEXT("ExplorationDangerZone"))})
 {
  TArray<AActor*> Targets; UGameplayStatics::GetAllActorsWithTag(World,Tag,Targets);
  for(AActor* Target : Targets) for(float Height : {0.f,500.f,1200.f})
  {
   const FVector Start(Target->GetActorLocation().X,Target->GetActorLocation().Y,Home.Z+Height);
   Rover->SetActorLocation(Start,false,nullptr,ETeleportType::TeleportPhysics);
   Rover->BeginExplorationControl(); Return->BeginNavigationPlan();
   for(int I=0; I<1800 && Return->State.Mode==EExplorationReturnMode::Planning; ++I) Return->TickNavigationPlan(1.f/120);
   AddInfo(FString::Printf(TEXT("Scene target=%s start=%s clear=%d mode=%d detail=%s"),*Target->GetName(),*Start.ToString(),Rover->IsUnderwaterNavigationSegmentClear(Start,Start),int(Return->State.Mode),*Return->State.Detail.ToString()));
   TestTrue(TEXT("Actual observation pose has a valid A* route"),Return->State.Mode==EExplorationReturnMode::Returning);
   Return->ResumeManual();
  }
 }
 Fixture.ForwardErrorMessages(this); return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplorationReadinessTest, "MqttVehicle.Exploration.ReturnReadiness",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FExplorationReadinessTest::RunTest(const FString& Parameters)
{
 const bool SavedUse=FApp::UseVRFocus(), SavedFocus=FApp::HasVRFocus();
 FApp::SetUseVRFocus(true); FApp::SetHasVRFocus(true);
 TestTrue(TEXT("Focused VR is playable without a focused desktop window"),MqttVehicleControl::HasFocus(false));
 FApp::SetHasVRFocus(false);
 TestFalse(TEXT("VR overlay focus loss blocks driving even if desktop is focused"),MqttVehicleControl::HasFocus(true));
 FApp::SetUseVRFocus(false);
 TestFalse(TEXT("Unfocused desktop blocks driving"),MqttVehicleControl::HasFocus(false));
 TestTrue(TEXT("Focused desktop allows driving"),MqttVehicleControl::HasFocus(true));
 FApp::SetUseVRFocus(SavedUse); FApp::SetHasVRFocus(SavedFocus);
 FTestWorldWrapper Fixture; if(!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
 auto* Bridge=NewObject<UMqttVehicleBridgeSubsystem>(Fixture.GetTestWorld());
 Bridge->bReceiveVehicleCommands=false; Bridge->bPublishVehicleInput=true; Bridge->bDashboardOutputEnabled=false;
 TestTrue(TEXT("RC disabled allows virtual return without MQTT"),Bridge->IsLocalReturnConnectionReady());
 Bridge->bDashboardOutputEnabled=true;
 TestFalse(TEXT("RC enabled still requires a live MQTT connection"),Bridge->IsLocalReturnConnectionReady());
 Bridge->bPublishVehicleInput=false;
 TestTrue(TEXT("Local-only PC return does not require MQTT"),Bridge->IsLocalReturnConnectionReady());
 Bridge->bReceiveVehicleCommands=true;
 TestFalse(TEXT("External control conflict remains blocked"),Bridge->IsLocalReturnConnectionReady());
 return !HasAnyErrors();
}

#endif
