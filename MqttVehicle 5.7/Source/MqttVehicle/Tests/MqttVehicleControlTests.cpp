// Regression coverage for pedal ordering and the signed command sent to the RC car.
#if WITH_DEV_AUTOMATION_TESTS

#include "MqttVehiclePawn.h"
#include "MqttVehicleUI.h"
#include "SubmersibleViewComponent.h"
#include "ExplorationRoute.h"
#include "Components/BoxComponent.h"
#include "GameFramework/WorldSettings.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputSubsystemInterface.h"
#include "InputKeyEventArgs.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/InputDelegateBinding.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "SubmersiblePropulsionComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/TextBlock.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"
#include <limits>

// Build the real key mappings without a desktop viewport or a live RC subsystem.
struct FIsolatedUnderwaterInput : IEnhancedInputSubsystemInterface
{
	UEnhancedPlayerInput* Input = nullptr;
	TMap<TObjectPtr<const UInputAction>, FInjectedInput> Continuous;
	virtual UEnhancedPlayerInput* GetPlayerInput() const override { return Input; }
	virtual TMap<TObjectPtr<const UInputAction>, FInjectedInput>& GetContinuouslyInjectedInputs() override { return Continuous; }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqttVehicleControlTest,
	"MqttVehicle.Control.ManualInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMqttVehicleControlTest::RunTest(const FString& Parameters)
{
	// GamePreview creates vehicle physics without the Game/PIE MQTT subsystem.
	FTestWorldWrapper Fixture;
	if (!TestTrue(TEXT("Create physics world"), Fixture.CreateTestWorld(EWorldType::GamePreview))) return false;
	UClass* PawnClass = LoadClass<AMqttVehiclePawn>(nullptr,
		TEXT("/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn.BP_SportsCar_Pawn_C"));
	if (!TestNotNull(TEXT("Load actual vehicle Blueprint"), PawnClass)) return false;
	AMqttVehiclePawn* Pawn = Fixture.GetTestWorld()->SpawnActor<AMqttVehiclePawn>(PawnClass);
	if (!TestNotNull(TEXT("Spawn vehicle"), Pawn)) return false;
	auto* Movement = Pawn->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!TestNotNull(TEXT("Vehicle movement"), Movement)) return false;
	Fixture.BeginPlayInTestWorld();
	Fixture.TickTestWorld();

	auto Check = [this, Pawn](const TCHAR* Label, double Th, double St)
	{
		TestTrue(Label, Pawn->GetVehicleControlInput().Equals(FVector2D(Th, St), 0.0001));
	};
	auto Reset = [Pawn, Movement]()
	{
		Pawn->StopVehicleInput();
		Movement->SetTargetGear(1, true);
	};
	Reset();
	Check(TEXT("Initial stop"), 0, 0);
	Pawn->DoSteering(0.4f); Pawn->DoThrottle(0.7f);
	Check(TEXT("Forward and steering"), 0.7, 0.4);
	Pawn->DoThrottle(0);
	Check(TEXT("Releasing throttle suppresses stationary pivot"), 0, 0);
	Pawn->DoThrottle(0.8f); Pawn->DoBrake(0.6f);
	Check(TEXT("Forward then brake stops"), 0, 0);
	Reset(); Pawn->DoBrake(0.6f); Pawn->DoThrottle(0.8f);
	Check(TEXT("Brake then forward also stops"), 0, 0);
	TestTrue(TEXT("Both pedals hold virtual handbrake"), Movement->GetHandbrakeInput());
	Pawn->DoBrakeStop();
	Check(TEXT("Releasing brake preserves held forward"), 0.8, 0);
	Pawn->DoHandbrakeStart();
	Check(TEXT("Handbrake stops RC command"), 0, 0);
	Pawn->DoHandbrakeStop();
	Check(TEXT("Release handbrake restores held forward"), 0.8, 0);

	Reset(); Movement->SetTargetGear(-1, true);
	TestEqual(TEXT("Physics vehicle accepts reverse gear"), Movement->GetTargetGear(), -1);
	Pawn->DoBrake(0.6f); Pawn->DoSteering(-0.25f);
	Check(TEXT("S pedal emits signed reverse"), -0.6, -0.25);
	Pawn->DoBrakeStop();
	Check(TEXT("Release reverse stops"), 0, 0);
	Pawn->DoThrottle(-0.5f);
	Check(TEXT("Signed throttle API reverses"), -0.5, -0.25);
	Pawn->StopVehicleInput();
	Check(TEXT("Forced stop clears held inputs"), 0, 0);
	Pawn->DoSteering(1);
	Check(TEXT("Steering cannot release forced stop"), 0, 0);
	Movement->SetTargetGear(1, true); Pawn->DoThrottle(5);
	Check(TEXT("Clamp out of range input"), 1, 1);
	Pawn->DoThrottle(std::numeric_limits<float>::quiet_NaN());
	Check(TEXT("Nonfinite throttle stops command"), 0, 0);
	Pawn->DoThrottle(1); Pawn->DoResetVehicle();
	Check(TEXT("Vehicle reset clears drive"), 0, 0);

	// Autonomous Unreal control must never leak into the physical RC command stream.
	Reset();
	Pawn->DoThrottle(0.7f);
	const bool bOriginalAutoGears = Movement->GetUseAutoGears();
	const bool bOriginalReverseAsBrake = Movement->bReverseAsBrake;
	Pawn->BeginExplorationControl();
	Pawn->ApplyExplorationControl(0.5f, 0.8f, 0.0f, false);
	TestTrue(TEXT("Pawn accepts the steered launch throttle"), FMath::IsNearlyEqual(Movement->GetThrottleInput(), 0.5f));
	Check(TEXT("Raised automatic throttle is excluded from MQTT"), 0, 0);
	Pawn->ApplyExplorationControl(-0.25f, 0.3f, 0.0f, false);
	Check(TEXT("Automatic reverse/steering is excluded from MQTT"), 0, 0);
	Pawn->DoThrottle(0.7f);
	TestTrue(TEXT("Already held throttle does not cancel discovery return"), Pawn->IsExplorationControlActive());
	Pawn->DoThrottle(0.0f);
	Pawn->DoThrottle(0.5f);
	TestFalse(TEXT("Released then pressed input takes manual control"), Pawn->IsExplorationControlActive());
	Check(TEXT("Manual takeover first stops old automatic output"), 0, 0);
	TestEqual(TEXT("Restore automatic gearbox setting"), Movement->GetUseAutoGears(), bOriginalAutoGears);
	TestEqual(TEXT("Restore pedal reverse policy"), bool(Movement->bReverseAsBrake), bOriginalReverseAsBrake);
	Movement->SetTargetGear(1, true);
	Pawn->DoThrottle(0.5f);
	Check(TEXT("New manual input can drive after takeover"), 0.5, 0);
	Pawn->BeginExplorationControl();
	Pawn->StopVehicleInput();
	Pawn->DoThrottle(0.5f);
	Check(TEXT("External stop cannot restart from held throttle"), 0, 0);
	Pawn->DoThrottle(0.0f);
	Pawn->DoThrottle(0.4f);
	Check(TEXT("Release then fresh manual input resumes after stop"), 0.4, 0);
	Pawn->BeginExplorationControl();
	Pawn->DoHandbrakeStart();
	TestFalse(TEXT("Handbrake immediately interrupts automatic control"), Pawn->IsExplorationControlActive());
	Check(TEXT("Handbrake interrupt publishes zero"), 0, 0);
	Pawn->EndExplorationControl(false);
	UClass* RoverClass = LoadClass<AMqttVehiclePawn>(nullptr,
		TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
	if (TestNotNull(TEXT("Selected underwater rover exists"), RoverClass))
	{
		AMqttVehiclePawn* Rover = Fixture.GetTestWorld()->SpawnActor<AMqttVehiclePawn>(RoverClass, FVector(1000,0,200), FRotator::ZeroRotator);
		Rover->DispatchBeginPlay(); // The GamePreview fixture only starts actors already present in BeginPlayInTestWorld.
		AActor* Home = Fixture.GetTestWorld()->SpawnActor<AActor>();
		Home->Tags.Add(TEXT("ExplorationReturnAnchor"));
		// AActor without a root stays at the origin; match the rover explicitly.
		Rover->SetActorLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
		TestTrue(TEXT("Playable Lanzaman hull is assigned"), Rover->HasSubmersibleChoices());
		TestEqual(TEXT("Lanzaman is the default hull"), Rover->GetSubmersibleVariant(), 1);
		TestTrue(TEXT("Begin in cockpit"), Rover->IsInteriorCameraActive());
		TestTrue(TEXT("Lanzaman camera is raised above the lower monitor"), Rover->GetFrontSpringArm()->GetRelativeLocation().Equals(FVector(-90,-62,320)));
		TestTrue(TEXT("Procedural cockpit exists"), Rover->SubmersibleView->GetCockpitPartCount() > 32);
		TArray<TWeakObjectPtr<UPrimitiveComponent>> Hidden;
		Rover->SubmersibleView->GetHiddenForView(true, Hidden);
		TestFalse(TEXT("Lanzaman interior retains original seats and monitors"), Hidden.Contains(Rover->SubmersibleHull.Get()));
		Hidden.Reset(); Rover->SubmersibleView->GetHiddenForView(false, Hidden);
		TestFalse(TEXT("External feed retains hull"), Hidden.Contains(Rover->SubmersibleHull.Get()));
		TestEqual(TEXT("External view hides all cockpit pieces"), Hidden.Num(), Rover->SubmersibleView->GetCockpitPartCount());
		Rover->DoLookAround(65.0f); Rover->DoLookUp(18.0f);
		const FRotator SharedLook = Rover->GetFrontSpringArm()->GetRelativeRotation();
		TestTrue(TEXT("Cockpit mouse look also rotates external feed"), SharedLook.Equals(Rover->GetBackSpringArm()->GetRelativeRotation()));
		Rover->DoToggleCamera();
		TestTrue(TEXT("Switch preserves shared viewing direction"), SharedLook.Equals(Rover->GetBackSpringArm()->GetRelativeRotation()));
		Rover->DoLookAround(-22.0f); Rover->DoLookUp(-7.0f);
		TestTrue(TEXT("Exterior mouse look also rotates cockpit feed"), Rover->GetFrontSpringArm()->GetRelativeRotation().Equals(Rover->GetBackSpringArm()->GetRelativeRotation()));
		Rover->DoRecenterCamera();
		TestTrue(TEXT("Recenter synchronizes both views"), Rover->GetFrontSpringArm()->GetRelativeRotation().Equals(Rover->GetBackSpringArm()->GetRelativeRotation()));
		TestTrue(TEXT("Tab API swaps main camera to exterior"), !Rover->IsInteriorCameraActive() && Rover->GetBackCamera()->IsActive() && !Rover->GetFollowCamera()->IsActive());
		Rover->DoToggleCamera();
		TestFalse(TEXT("Removed Iris choice is rejected even at home"), Rover->SelectSubmersible(0));
		TestEqual(TEXT("Rejected Iris choice retains Lanzaman"), Rover->GetSubmersibleVariant(), 1);
		TestTrue(TEXT("Stopped at home can choose Lanzaman"), Rover->SelectSubmersible(1));
		TestTrue(TEXT("Reselecting Lanzaman restores the raised viewpoint"), Rover->GetFrontSpringArm()->GetRelativeLocation().Equals(FVector(-90,-62,320)));
		// Exercise the actual asset action and both native/Blueprint delegates together.
		auto* Mapping = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/VehicleTemplate/Input/IMC_Vehicle_Default.IMC_Vehicle_Default"));
		const UInputAction* CameraAction = nullptr;
		if (Mapping) for (const auto& Key : Mapping->GetMappings()) if (Key.Key == EKeys::Tab) CameraAction = Key.Action;
		if (TestNotNull(TEXT("Tab has a saved camera action"), CameraAction))
		{
			auto* TestInput = NewObject<UEnhancedInputComponent>(Rover);
			Rover->SetupPlayerInputComponent(TestInput);
			UInputDelegateBinding::BindInputDelegates(Rover->GetClass(), TestInput, Rover);
			auto* InputOwner = Fixture.GetTestWorld()->SpawnActor<APlayerController>();
			auto* PlayerInput = NewObject<UEnhancedPlayerInput>(InputOwner);
			const TArray<UInputComponent*> Stack{TestInput};
			PlayerInput->InjectInputForAction(CameraAction, FInputActionValue(true));
			PlayerInput->ProcessInputStack(Stack, 1.0f/60.0f, false);
			TestFalse(TEXT("One camera press changes to exterior exactly once"), Rover->IsInteriorCameraActive());
			PlayerInput->InjectInputForAction(CameraAction, FInputActionValue(true));
			PlayerInput->ProcessInputStack(Stack, 1.0f/60.0f, false);
			TestFalse(TEXT("Holding camera key does not toggle again"), Rover->IsInteriorCameraActive());
			PlayerInput->InjectInputForAction(CameraAction, FInputActionValue(false));
			PlayerInput->ProcessInputStack(Stack, 1.0f/60.0f, false);
			PlayerInput->InjectInputForAction(CameraAction, FInputActionValue(true));
			PlayerInput->ProcessInputStack(Stack, 1.0f/60.0f, false);
			TestTrue(TEXT("Second camera press restores cockpit"), Rover->IsInteriorCameraActive());
			PlayerInput->FlushPressedKeys();
		}
		TestTrue(TEXT("Selected mesh matches Lanzaman"), Rover->SubmersibleHull->GetStaticMesh() == Rover->LanzamanSubmersible.Get());
		Rover->DoThrottle(0.5f);
		TestFalse(TEXT("Held driving input blocks selection"), Rover->SelectSubmersible(1));
		Rover->StopVehicleInput();
		Rover->DoThrottle(0.0f);
		Rover->SetActorLocation(FVector(1000,0,0),false,nullptr,ETeleportType::TeleportPhysics);
		TestFalse(TEXT("Selection outside the departure pad is blocked"), Rover->SelectSubmersible(1));
		TestTrue(TEXT("Underwater forward limit is six actual MPH"), FMath::IsNearlyEqual(Rover->ManualForwardSpeedLimit, 6.0f * 1.609344f, 0.0001f));
		TestEqual(TEXT("Underwater reverse limit is three km/h"), Rover->ManualReverseSpeedLimit, 3.0f);
		TestTrue(TEXT("Return cruises at five MPH independently of manual six MPH"), FMath::IsNearlyEqual(Rover->GetReturnCruiseSpeed(), 5.0f * 44.704f, 0.001f));
		Rover->BeginExplorationControl();
		const FVector PivotStart = Rover->GetActorLocation();
		const FVector PivotForward = Rover->GetActorForwardVector();
		for (int32 I=0; I<36; ++I) TestTrue(TEXT("Local pivot step is clear"), Rover->PivotInPlace(5.0f));
		TestTrue(TEXT("180-degree pivot has no translation"), Rover->GetActorLocation().Equals(PivotStart,0.01));
		TestTrue(TEXT("180-degree pivot faces backwards"), FVector::DotProduct(Rover->GetActorForwardVector(),PivotForward)<-0.999);
		TestTrue(TEXT("Automatic pivot stays excluded from MQTT"), Rover->GetVehicleControlInput().IsNearlyZero());
		Rover->EndExplorationControl(false);
		Rover->DoSteering(-1.0f);
		const double BeforeYaw = Rover->GetActorRotation().Yaw;
		Rover->Tick(0.1f);
		TestTrue(TEXT("Steering alone rotates left"), FMath::FindDeltaAngleDegrees(BeforeYaw,Rover->GetActorRotation().Yaw)<-4.0);
		TestTrue(TEXT("Manual left pivot emits steering without throttle"), Rover->GetVehicleControlInput().Equals(FVector2D(0,-1)) && Rover->GetChaosVehicleMovement()->GetThrottleInput()==0.0f);
		Rover->StopVehicleInput(); Rover->DoSteering(-1.0f);
		const FRotator StoppedRotation = Rover->GetActorRotation(); Rover->Tick(0.1f);
		TestTrue(TEXT("Held steering cannot undo explicit stop"), Rover->GetActorRotation().Equals(StoppedRotation));
		Rover->DoSteering(0.0f); Rover->DoSteering(1.0f); Rover->Tick(0.1f);
		TestFalse(TEXT("Fresh steering resumes after release"), Rover->GetActorRotation().Equals(StoppedRotation));
	}
	UClass* UIClass = LoadClass<UObject>(nullptr, TEXT("/Game/VehicleTemplate/Blueprints/UI/BP_VehicleAdvUI.BP_VehicleAdvUI_C"));
	if (TestNotNull(TEXT("Speed HUD exists"), UIClass))
	{
		const FBoolProperty* Units = FindFProperty<FBoolProperty>(UIClass, TEXT("bIsMPH"));
		if (TestNotNull(TEXT("Speed HUD unit setting"), Units))
			TestTrue(TEXT("Displayed MPH label matches the calculation"), Units->GetPropertyValue_InContainer(UIClass->GetDefaultObject()));
		UMqttVehicleUI* UI = CreateWidget<UMqttVehicleUI>(Fixture.GetTestWorld(), UIClass);
		if (TestNotNull(TEXT("Instantiate speed HUD"), UI))
		{
			UI->UpdateSpeed(6.0f * 44.704f);
			UTextBlock* Label = Cast<UTextBlock>(UI->GetWidgetFromName(TEXT("UnitLabel")));
			if (TestNotNull(TEXT("HUD has visible unit label"), Label))
				TestEqual(TEXT("Runtime unit label agrees with MPH calculation"), Label->GetText().ToString(), FString(TEXT("MPH")));
		}
	}
	Fixture.ForwardErrorMessages(this);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqttVehicleCruiseTest,
	"MqttVehicle.Control.FiveMphCruise", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMqttVehicleCruiseTest::RunTest(const FString& Parameters)
{
	// An isolated flat physics world with no controller, MQTT subsystem or external endpoint.
	FTestWorldWrapper Fixture;
	if (!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
	UWorld* World = Fixture.GetTestWorld();
	AActor* Ground = World->SpawnActor<AActor>();
	auto* Floor = NewObject<UBoxComponent>(Ground);
	Ground->SetRootComponent(Floor);
	Floor->SetBoxExtent(FVector(10000,10000,50));
	Floor->SetCollisionProfileName(TEXT("BlockAll"));
	Floor->RegisterComponent();
	Ground->SetActorLocation(FVector(0,0,-50));
	UClass* RoverClass = LoadClass<AMqttVehiclePawn>(nullptr,
		TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
	if (!TestNotNull(TEXT("Load saved rover for cruise physics"), RoverClass)) return false;
	auto* Rover = World->SpawnActor<AMqttVehiclePawn>(RoverClass, FVector(0,0,35), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Spawn rover for cruise physics"), Rover)) return false;
	Rover->bEnableUnderwaterMovement = false; // Retain coverage of the legacy wheeled mode.
	Fixture.BeginPlayInTestWorld();
	World->GetWorldSettings()->NotifyBeginPlay(); // GamePreview has no GameMode to dispatch this.
	Rover->StopVehicleInput();
	for (int32 I=0; I<120; ++I) Fixture.TickTestWorld(1.0f/60.0f);
	Rover->BeginExplorationControl();
	float Peak = 0, CruiseSum = 0;
	for (int32 I=0; I<600; ++I)
	{
		const auto Command = ExplorationRoute::Follow(Rover->GetActorLocation(), Rover->GetActorForwardVector(),
			FVector(6000,0,0), 1, Rover->GetChaosVehicleMovement()->GetForwardSpeed(), Rover->GetReturnCruiseSpeed());
		Rover->ApplyExplorationControl(Command.Throttle, Command.Steering, Command.Brake, false);
		Fixture.TickTestWorld(1.0f/60.0f);
		const float Mph = Rover->GetChaosVehicleMovement()->GetForwardSpeed()/44.704f;
		Peak = FMath::Max(Peak,Mph);
		if (I>=300) CruiseSum += Mph;
	}
	AddInfo(FString::Printf(TEXT("Offline Chaos cruise: peak %.3f MPH, final-five-second mean %.3f MPH, distance %.1f cm"), Peak,CruiseSum/300,Rover->GetActorLocation().X));
	TestTrue(TEXT("Actual flat-ground cruise reaches five MPH"), Peak >= 4.8f && Peak <= 5.2f);
	TestTrue(TEXT("Cruise sustains the target rather than three MPH"), CruiseSum/300 >= 4.7f && CruiseSum/300 <= 5.2f);
	TestTrue(TEXT("Automatic physics movement has zero MQTT command"), Rover->GetVehicleControlInput().IsNearlyZero());
	Rover->StopVehicleInput();
	Fixture.ForwardErrorMessages(this);
	return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMqttUnderwaterMovementTest,
	"MqttVehicle.Control.UnderwaterMovement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMqttUnderwaterMovementTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper Fixture;
	if (!Fixture.CreateTestWorld(EWorldType::GamePreview)) return false;
	UWorld* World = Fixture.GetTestWorld();
	UClass* RoverClass = LoadClass<AMqttVehiclePawn>(nullptr,
		TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
	if (!TestNotNull(TEXT("Load underwater rover"), RoverClass)) return false;
	auto* Rover = World->SpawnActor<AMqttVehiclePawn>(RoverClass, FVector(0,0,100), FRotator::ZeroRotator);
	Fixture.BeginPlayInTestWorld(); World->GetWorldSettings()->NotifyBeginPlay();
	TestTrue(TEXT("Saved exploration hull enables underwater movement"), Rover->IsUnderwaterMovementEnabled());
	TestFalse(TEXT("Swimming does not depend on wheel traction or gravity"), Rover->GetMesh()->IsSimulatingPhysics());
	auto* Input = NewObject<UEnhancedInputComponent>(Rover);
	TStrongObjectPtr<UEnhancedInputComponent> KeepInput(Input);
	Rover->SetupPlayerInputComponent(Input);
	const auto* MappingProperty = FindFProperty<FObjectProperty>(Rover->GetClass(), TEXT("ObservationMappingContext"));
	auto* Mapping = MappingProperty ? Cast<UInputMappingContext>(MappingProperty->GetObjectPropertyValue_InContainer(Rover)) : nullptr;
	if (!TestNotNull(TEXT("Runtime mapping exists even in isolated input test"), Mapping)) return false;
	const UInputAction* Up = nullptr; const UInputAction* Down = nullptr;
	for (const auto& Entry : Mapping->GetMappings())
	{
		if (Entry.Key == EKeys::E) Up = Entry.Action;
		if (Entry.Key == EKeys::Q) Down = Entry.Action;
	}
	if (!TestNotNull(TEXT("E maps to ascent"), Up) || !TestNotNull(TEXT("Q maps to descent"), Down)) return false;
	auto* Owner = World->SpawnActor<APlayerController>();
	auto* PlayerInput = NewObject<UEnhancedPlayerInput>(Owner);
	TStrongObjectPtr<UEnhancedPlayerInput> KeepPlayerInput(PlayerInput);
	Owner->SetActorTickEnabled(false); // This isolated stack is pumped explicitly below.
	FIsolatedUnderwaterInput InputSubsystem;
	InputSubsystem.Input = PlayerInput;
	FModifyContextOptions Options; Options.bForceImmediately = true;
	InputSubsystem.AddMappingContext(Mapping, 10, Options);
	// Reproduce a lower-priority template grip/handbrake conflict.
	const auto* HandbrakeProperty = FindFProperty<FObjectProperty>(Rover->GetClass(), TEXT("HandbrakeAction"));
	auto* Handbrake = HandbrakeProperty ? Cast<UInputAction>(HandbrakeProperty->GetObjectPropertyValue_InContainer(Rover)) : nullptr;
	if (!TestNotNull(TEXT("Template handbrake action exists"), Handbrake)) return false;
	auto* LowerPriorityMapping = NewObject<UInputMappingContext>(Owner);
	TStrongObjectPtr<UInputMappingContext> KeepLowerPriorityMapping(LowerPriorityMapping);
	LowerPriorityMapping->MapKey(Handbrake, EKeys::OculusTouch_Right_Grip_Click);
	InputSubsystem.AddMappingContext(LowerPriorityMapping, 0, Options);
	const TArray<UInputComponent*> Stack{Input};
	bool bUpPressed=false, bDownPressed=false;
	FKey UpKey = EKeys::E, DownKey = EKeys::Q;
	auto Step = [&](float Ascend, float Descend, int32 Frames)
	{
		auto Key = [&](FKey Code, bool bPressed, bool& bPrevious)
		{
			if (bPressed != bPrevious)
				PlayerInput->InputKey(FInputKeyEventArgs(nullptr, INPUTDEVICEID_NONE, Code,
					bPressed ? IE_Pressed : IE_Released, bPressed ? 1.0f : 0.0f, false, FPlatformTime::Cycles64()));
			bPrevious=bPressed;
		};
		Key(UpKey, Ascend>0, bUpPressed); Key(DownKey, Descend>0, bDownPressed);
		for (int32 I=0; I<Frames; ++I)
		{
			PlayerInput->ProcessInputStack(Stack, 1.0f/60, false);
			Fixture.TickTestWorld(1.0f/60);
		}
	};
	const FVector Start = Rover->GetActorLocation();
	Step(1,0,180);
	const double RaisedZ = Rover->GetActorLocation().Z;
	TestTrue(TEXT("E action raises hull more than two metres in three seconds"), RaisedZ > Start.Z+200);
	TestTrue(TEXT("Vertical input alone never drives RC"), Rover->GetVehicleControlInput().IsNearlyZero());
	Step(0,0,120);
	TestTrue(TEXT("Release E holds depth without gravity drift"), FMath::Abs(Rover->GetActorLocation().Z-RaisedZ)<0.1);
	Step(1,1,60);
	TestTrue(TEXT("Holding Q and E together cancels vertical movement"), FMath::Abs(Rover->GetActorLocation().Z-RaisedZ)<0.1);
	Step(0,1,60);
	TestTrue(TEXT("Q action descends"), Rover->GetActorLocation().Z < RaisedZ-60);
	Rover->StopVehicleInput(); const FVector Stopped=Rover->GetActorLocation();
	Step(0,1,60);
	TestTrue(TEXT("Held Q cannot undo explicit stop"), Rover->GetActorLocation().Equals(Stopped,0.1));
	Step(0,0,2); Step(1,0,60);
	TestTrue(TEXT("Release and fresh E resumes"), Rover->GetActorLocation().Z > Stopped.Z+60);
	Rover->DoHandbrakeStart(); const double BrakeZ=Rover->GetActorLocation().Z;
	Rover->DoHandbrakeStop(); Step(1,0,30);
	TestTrue(TEXT("Space requires held E to be released"), FMath::Abs(Rover->GetActorLocation().Z-BrakeZ)<0.1);
	Step(0,0,2);
	UpKey = EKeys::OculusTouch_Right_A_Click;
	DownKey = EKeys::OculusTouch_Right_B_Click;
	const double BeforeQuestZ = Rover->GetActorLocation().Z;
	Step(1,0,60);
	const double QuestRaisedZ = Rover->GetActorLocation().Z;
	TestTrue(TEXT("Quest right A raises hull"), QuestRaisedZ > BeforeQuestZ+60);
	Step(0,0,60);
	TestTrue(TEXT("Release Quest A holds depth"), FMath::Abs(Rover->GetActorLocation().Z-QuestRaisedZ)<0.1);
	Step(1,1,60);
	TestTrue(TEXT("Quest A and B together cancel"), FMath::Abs(Rover->GetActorLocation().Z-QuestRaisedZ)<0.1);
	Step(0,1,60);
	TestTrue(TEXT("Quest right B lowers hull"), Rover->GetActorLocation().Z < QuestRaisedZ-60);
	TestTrue(TEXT("Quest vertical input never drives RC"), Rover->GetVehicleControlInput().IsNearlyZero());
	Rover->StopVehicleInput();
	const FVector QuestStopped = Rover->GetActorLocation();
	Step(0,1,60);
	TestTrue(TEXT("Held Quest B cannot undo explicit stop"), Rover->GetActorLocation().Equals(QuestStopped,0.1));
	Step(0,0,2); Rover->DoThrottle(1); Step(0,0,180);
	TestTrue(TEXT("Propulsion works above the seabed"), Rover->GetActorLocation().X>600);
	TestTrue(TEXT("Underwater manual cruise reaches six MPH"), FMath::Abs(Rover->GetDriveForwardSpeed()/44.704f-6)<0.05f);
	TestTrue(TEXT("Underwater forward maps manual throttle"), Rover->GetVehicleControlInput().Equals(FVector2D(1,0)));
	Rover->DoSteering(.4f);
	TestTrue(TEXT("Underwater steering accompanies propulsion"), Rover->GetVehicleControlInput().Equals(FVector2D(1,.4),.0001));
	Rover->DoBrake(.6f);
	TestTrue(TEXT("Conflicting underwater pedals stop RC"), Rover->GetVehicleControlInput().IsNearlyZero());
	Rover->DoThrottle(0);
	TestTrue(TEXT("Underwater reverse does not depend on Chaos gear"), Rover->GetVehicleControlInput().Equals(FVector2D(-.6,.4),.0001));
	Rover->DoBrakeStop();
	TestTrue(TEXT("Steering alone emits manual pivot"), Rover->GetVehicleControlInput().Equals(FVector2D(0,.4),.0001));
	Rover->DoSteering(-1);
	TestTrue(TEXT("Left pivot sends negative steering"), Rover->GetVehicleControlInput().Equals(FVector2D(0,-1)));
	Rover->DoSteering(1);
	TestTrue(TEXT("Right pivot sends positive steering"), Rover->GetVehicleControlInput().Equals(FVector2D(0,1)));
	Rover->DoSteering(0);
	Rover->DoThrottle(.5f); Rover->DoHandbrakeStart();
	TestTrue(TEXT("Underwater handbrake stops RC"), Rover->GetVehicleControlInput().IsNearlyZero());
	Rover->DoHandbrakeStop(); Rover->DoThrottle(0);
	Rover->DoThrottle(0); Step(0,0,2);
	const bool bInitialInterior = Rover->IsInteriorCameraActive();
	Rover->DoThrottle(.5f);
	auto Grip = [&](bool bPressed)
	{
		PlayerInput->InputKey(FInputKeyEventArgs(nullptr, INPUTDEVICEID_NONE, EKeys::OculusTouch_Right_Grip_Click,
			bPressed ? IE_Pressed : IE_Released, bPressed ? 1.0f : 0.0f, true, FPlatformTime::Cycles64()));
		Step(0,0,2);
	};
	Grip(true);
	TestTrue(TEXT("Right grip switches camera"), Rover->IsInteriorCameraActive() != bInitialInterior);
	TestTrue(TEXT("Right grip does not apply template handbrake"), Rover->GetVehicleControlInput().Equals(FVector2D(.5,0)));
	Step(0,0,30);
	TestTrue(TEXT("Held grip does not repeatedly toggle camera"), Rover->IsInteriorCameraActive() != bInitialInterior);
	Grip(false);
	TestTrue(TEXT("Grip release does not toggle camera"), Rover->IsInteriorCameraActive() != bInitialInterior);
	Grip(true);
	TestTrue(TEXT("Next grip press returns to original camera"), Rover->IsInteriorCameraActive() == bInitialInterior);
	Grip(false); Rover->DoThrottle(0); Step(0,0,2);
	Rover->BeginExplorationControl();
	const FVector Target=Rover->GetActorLocation()+FVector(1000,0,400);
	float Peak=0;
	float PeakMotorThrottle=0;
	for (int32 I=0; I<600; ++I)
	{
		Rover->ApplyUnderwaterReturnTarget(Target,Rover->GetReturnCruiseSpeed());
		Fixture.TickTestWorld(1.0f/60);
		Peak=FMath::Max(Peak,static_cast<float>(Rover->GetVelocity().Size()/44.704));
		PeakMotorThrottle=FMath::Max(PeakMotorThrottle,static_cast<float>(Rover->GetAutomaticVehicleControlInput().X));
	}
	TestTrue(TEXT("Automatic swim tracks horizontal and vertical target"), FVector::Dist(Rover->GetActorLocation(),Target)<5);
	TestTrue(TEXT("Automatic swim reaches five MPH"), Peak>4.9f && Peak<=5.01f);
	TestTrue(TEXT("Automatic horizontal return produces bounded motor throttle"), PeakMotorThrottle>0.9f && PeakMotorThrottle<=1.0f);
	Rover->ApplyExplorationControl(0,0,1,true);
	TestTrue(TEXT("Return hold clears motor command"), Rover->GetAutomaticVehicleControlInput().IsNearlyZero());
	TestTrue(TEXT("Automatic left pivot succeeds"), Rover->PivotInPlace(-1));
	TestTrue(TEXT("Automatic left pivot mirrors direction"), Rover->GetAutomaticVehicleControlInput().Equals(FVector2D(0,-1)));
	TestTrue(TEXT("Automatic right pivot succeeds"), Rover->PivotInPlace(1));
	TestTrue(TEXT("Automatic right pivot mirrors direction"), Rover->GetAutomaticVehicleControlInput().Equals(FVector2D(0,1)));
	Rover->StopVehicleInput();
	TestTrue(TEXT("Stop cancels automatic motor command immediately"), Rover->GetAutomaticVehicleControlInput().IsNearlyZero());
	Rover->BeginExplorationControl();
	Step(1,0,1);
	TestFalse(TEXT("Fresh vertical input cancels automatic return"), Rover->IsExplorationControlActive());
	Rover->EndExplorationControl(false); Step(0,0,2);
	const FVector BeforeWall=Rover->GetActorLocation();
	AActor* Wall=World->SpawnActor<AActor>(); auto* Box=NewObject<UBoxComponent>(Wall);
	Wall->SetRootComponent(Box); Box->SetBoxExtent(FVector(30,1000,2000));
	Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
	Wall->SetActorLocation(BeforeWall+FVector(600,0,0));
	Rover->DoThrottle(1); Step(0,0,180);
	TestTrue(TEXT("Full hull sweep stops before wall"), Rover->GetActorLocation().X<BeforeWall.X+450);
	Rover->DoThrottle(0); Step(0,0,2);
	Wall->SetActorLocation(FVector(100000,0,0));
	AActor* FloorActor=World->SpawnActor<AActor>(); auto* Floor=NewObject<UBoxComponent>(FloorActor);
	FloorActor->SetRootComponent(Floor); Floor->SetBoxExtent(FVector(10000,10000,50));
	Floor->SetCollisionProfileName(TEXT("BlockAll")); Floor->RegisterComponent();
	FloorActor->SetActorLocation(FVector(0,0,-50));
	Step(0,1,1800);
	TestTrue(TEXT("Descending cannot pass through seabed"), Rover->GetActorLocation().Z>-100);
	const double FloorZ=Rover->GetActorLocation().Z; Step(0,1,120);
	TestTrue(TEXT("Continued Q remains against seabed"), FMath::Abs(Rover->GetActorLocation().Z-FloorZ)<0.1);
	Step(0,0,2); Step(1,0,2400);
	TestTrue(TEXT("Ceiling bounds ascent to thirty metres above spawn"), FMath::Abs(Rover->GetActorLocation().Z-(Start.Z+3000))<0.1);
	Step(0,0,2); Step(0,1,60);
	TestTrue(TEXT("Q can leave upper limit"), Rover->GetActorLocation().Z<Start.Z+2940);
	AddInfo(FString::Printf(TEXT("E rise %.1f cm/3 s; automatic peak %.3f MPH; floor Z %.1f cm"),RaisedZ-Start.Z,Peak,FloorZ));
	Rover->StopVehicleInput(); Fixture.ForwardErrorMessages(this);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPropulsionEffectsTest,"MqttVehicle.Effects.Propulsion",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPropulsionEffectsTest::RunTest(const FString& Parameters)
{
 FTestWorldWrapper Fixture; Fixture.CreateTestWorld(EWorldType::GamePreview); Fixture.BeginPlayInTestWorld();
 UClass* Class=LoadClass<AMqttVehiclePawn>(nullptr,TEXT("/Game/Underwater/Vehicles/BP_ExplorationRover.BP_ExplorationRover_C"));
 auto* Rover=Fixture.GetTestWorld()->SpawnActor<AMqttVehiclePawn>(Class,FVector(0,0,800),FRotator::ZeroRotator);
 Rover->DispatchBeginPlay(); auto* FX=Rover->PropulsionEffects.Get();
 if(!TestNotNull(TEXT("Propulsion component attached"),FX)) return false;
 TestNotNull(TEXT("Bubble material saved and referenced"),FX->BubbleMaterial.Get());
 TestEqual(TEXT("Two rotors are built"),FX->Rotors.Num(),2);
 for(int I=0;I<60;++I) FX->UpdateEffects(1.0f/60);
 TestEqual(TEXT("Idle produces no bubbles"),FX->GetLiveBubbleCount(),0);
 for(int I=0;I<120;++I) { Rover->AddActorWorldOffset(FVector(3,0,0)); FX->UpdateEffects(1.0f/60); }
 TestTrue(TEXT("Motion spins both rotors and emits a bounded bubble pool"),FX->GetActivity()>.5 && FX->GetRotorAngle()!=0 && FX->GetLiveBubbleCount()>20 && FX->GetLiveBubbleCount()<=128);
 const FVector Trail=FX->Pool[0].Position; Rover->AddActorWorldOffset(FVector(20,0,0)); FX->UpdateEffects(1.0f/60);
 TestTrue(TEXT("Emitted bubbles stay in world space rather than attach to the moving hull"),FVector::Dist(Trail,FX->Pool[0].Position)<5);
 for(int I=0;I<240;++I) FX->UpdateEffects(1.0f/60);
 TestEqual(TEXT("Stop winds down all bubbles"),FX->GetLiveBubbleCount(),0);
 const float StoppedAngle=FX->GetRotorAngle(); FX->UpdateEffects(.1f);
 TestEqual(TEXT("Stopped propeller remains stationary"),FX->GetRotorAngle(),StoppedAngle);
 for(int I=0;I<60;++I) { Rover->AddActorWorldOffset(FVector(0,0,1)); FX->UpdateEffects(1.0f/60); }
 TestTrue(TEXT("Vertical movement also emits bubbles"),FX->GetLiveBubbleCount()>0);
 TestTrue(TEXT("Cosmetic effects never request motor control"),Rover->GetVehicleControlInput().IsNearlyZero());
 Fixture.ForwardErrorMessages(this); return !HasAnyErrors();
}

#endif
