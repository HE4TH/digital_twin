// Copyright Epic Games, Inc. All Rights Reserved.

#include "MqttVehiclePawn.h"
#include "SubmersibleViewComponent.h"
#include "SubmersiblePropulsionComponent.h"
#include "ExplorationRoute.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "MqttVehicleWheelFront.h"
#include "MqttVehicleWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MqttVehicle.h"
#include "TimerManager.h"
#include "HeadMountedDisplayFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

AMqttVehiclePawn::AMqttVehiclePawn()
{
	PropulsionEffects = CreateDefaultSubobject<USubmersiblePropulsionComponent>(TEXT("PropulsionEffects"));
	SubmersibleView = CreateDefaultSubobject<USubmersibleViewComponent>(TEXT("SubmersibleView"));
	SubmersibleHull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SubmersibleHull"));
	SubmersibleHull->SetupAttachment(GetMesh());
	SubmersibleHull->SetCollisionProfileName(TEXT("NoCollision"));
	SubmersibleHull->SetGenerateOverlapEvents(false);
	SubmersibleHull->SetCanEverAffectNavigation(false);
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void AMqttVehiclePawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AMqttVehiclePawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Canceled, this, &AMqttVehiclePawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AMqttVehiclePawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Canceled, this, &AMqttVehiclePawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AMqttVehiclePawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMqttVehiclePawn::StopBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Canceled, this, &AMqttVehiclePawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AMqttVehiclePawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMqttVehiclePawn::StopHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Canceled, this, &AMqttVehiclePawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::LookAround);
		SetupObservationInput(EnhancedInputComponent);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AMqttVehiclePawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogMqttVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMqttVehiclePawn::SetupObservationInput(UEnhancedInputComponent* Input)
{
	RemoveObservationInput();
	const APlayerController* PC = Cast<APlayerController>(GetController());
	ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	auto* Subsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer) : nullptr;

	// Transient mappings add pitch/recenter without editing or duplicating the asset yaw mapping.
	ObservationMappingContext = NewObject<UInputMappingContext>(this);
	ObservationPitchAction = NewObject<UInputAction>(this);
	ObservationPitchAction->ValueType = EInputActionValueType::Axis1D;
	ObservationRecenterAction = NewObject<UInputAction>(this);
	ObservationRecenterAction->ValueType = EInputActionValueType::Boolean;

	auto& MousePitch = ObservationMappingContext->MapKey(ObservationPitchAction, EKeys::MouseY);
	auto* MouseScale = NewObject<UInputModifierScalar>(ObservationMappingContext);
	MouseScale->Scalar = FVector(FMath::Max(MousePitchSensitivity, 0.01f));
	MousePitch.Modifiers.Add(MouseScale);

	auto& StickPitch = ObservationMappingContext->MapKey(ObservationPitchAction, EKeys::Gamepad_RightY);
	auto* DeadZone = NewObject<UInputModifierDeadZone>(ObservationMappingContext);
	DeadZone->LowerThreshold = 0.15f;
	DeadZone->UpperThreshold = 1.0f;
	StickPitch.Modifiers.Add(DeadZone);
	auto* StickScale = NewObject<UInputModifierScalar>(ObservationMappingContext);
	StickScale->Scalar = FVector(FMath::Max(GamepadPitchDegreesPerSecond, 1.0f));
	StickPitch.Modifiers.Add(StickScale);
	StickPitch.Modifiers.Add(NewObject<UInputModifierScaleByDeltaTime>(ObservationMappingContext));

	ObservationMappingContext->MapKey(ObservationRecenterAction, EKeys::R);
	ObservationMappingContext->MapKey(ObservationRecenterAction, EKeys::Gamepad_RightThumbstick);
	Input->BindAction(ObservationPitchAction, ETriggerEvent::Triggered, this, &AMqttVehiclePawn::LookUp);
	Input->BindAction(ObservationRecenterAction, ETriggerEvent::Started, this, &AMqttVehiclePawn::RecenterCamera);
	if (bEnableUnderwaterMovement && HasSubmersibleChoices())
	{
		// Consume the right grip before the vehicle template's handbrake mapping.
		ObservationToggleCameraAction = NewObject<UInputAction>(this, TEXT("UnderwaterToggleCamera"));
		ObservationToggleCameraAction->ValueType = EInputActionValueType::Boolean;
		ObservationToggleCameraAction->bConsumeInput = true;
		ObservationMappingContext->MapKey(ObservationToggleCameraAction, EKeys::OculusTouch_Right_Grip_Click);
		Input->BindAction(ObservationToggleCameraAction, ETriggerEvent::Started, this, &AMqttVehiclePawn::ToggleCamera);
		AscendAction = NewObject<UInputAction>(this, TEXT("UnderwaterAscend"));
		DescendAction = NewObject<UInputAction>(this, TEXT("UnderwaterDescend"));
		AscendAction->ValueType = DescendAction->ValueType = EInputActionValueType::Axis1D;
		ObservationMappingContext->MapKey(AscendAction, EKeys::E);
		ObservationMappingContext->MapKey(DescendAction, EKeys::Q);
		ObservationMappingContext->MapKey(AscendAction, EKeys::OculusTouch_Right_A_Click);
		ObservationMappingContext->MapKey(DescendAction, EKeys::OculusTouch_Right_B_Click);
		for (ETriggerEvent Event : {ETriggerEvent::Triggered, ETriggerEvent::Completed, ETriggerEvent::Canceled})
		{
			Input->BindAction(AscendAction, Event, this, &AMqttVehiclePawn::Ascend);
			Input->BindAction(DescendAction, Event, this, &AMqttVehiclePawn::Descend);
		}
	}
	if (Subsystem) Subsystem->AddMappingContext(ObservationMappingContext, 10);
	ObservationInputSubsystem = Subsystem;
}

void AMqttVehiclePawn::RemoveObservationInput()
{
	if (auto* Subsystem = ObservationInputSubsystem.Get(); Subsystem && ObservationMappingContext)
	{
		Subsystem->RemoveMappingContext(ObservationMappingContext);
	}
	ObservationInputSubsystem.Reset();
}

void AMqttVehiclePawn::UnPossessed()
{
	StopVehicleInput();
	RemoveObservationInput();
	Super::UnPossessed();
}

void AMqttVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	bUnderwaterMovementActive = bEnableUnderwaterMovement && HasSubmersibleChoices();
	if (bUnderwaterMovementActive)
	{
		UnderwaterCeilingZ = GetActorLocation().Z + MaximumRiseAboveStart;
		ChaosVehicleMovement->SetComponentTickEnabled(false);
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetEnableGravity(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetActorRotation(FRotator(0, GetActorRotation().Yaw, 0));
	}
	if (HasSubmersibleChoices())
	{
		SubmersibleVariant = 1; // Lanzaman is the only playable exploration hull.
		SubmersibleHull->SetStaticMesh(LanzamanSubmersible);
		FrontSpringArm->SetRelativeLocation(FVector(-90,-62,320));
		SubmersibleView->RefreshInterior();
		FrontSpringArm->bEnableCameraRotationLag = BackSpringArm->bEnableCameraRotationLag = false;
		FrontCamera->SetFieldOfView(90.0f);
		bFrontCameraActive = true;
		FrontCamera->SetActive(true);
		BackCamera->SetActive(false);
	}

	FrontCameraHomeRotation = FrontSpringArm->GetRelativeRotation();
	BackCameraHomeRotation = BackSpringArm->GetRelativeRotation();
	FrontCameraHomeRotation.Roll = BackCameraHomeRotation.Roll = 0.0f;
	if (HasSubmersibleChoices())
	{
		BackCameraHomeRotation = FrontCameraHomeRotation;
		SetObservationRotation(FrontCameraHomeRotation);
	}
	// Keep observation pitch independent of the vehicle tilting on slopes.
	FrontSpringArm->bInheritPitch = BackSpringArm->bInheritPitch = false;
	FrontSpringArm->bInheritRoll = BackSpringArm->bInheritRoll = false;
	BackSpringArm->bDoCollisionTest = bUseObservationCameraCollision;
	MarkCameraInput();

	// set up the flipped check timer
	GetWorld()->GetTimerManager().SetTimer(FlipCheckTimer, this, &AMqttVehiclePawn::FlippedCheck, FlipCheckTime, true);
}

void AMqttVehiclePawn::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	RemoveObservationInput();
	StopVehicleInput();
	// clear the flipped check timer
	GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);

	Super::EndPlay(EndPlayReason);
}

void AMqttVehiclePawn::Tick(float Delta)
{
	Super::Tick(Delta);
	InitializeVRCockpit();
	if (bAllowStationaryTurn)
	{
		const auto* PC = Cast<APlayerController>(GetController());
		const UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
		if (PC && PC->IsLocalPlayerController() && (PC->IsMoveInputIgnored() || GetWorld()->IsPaused()
			|| !MqttVehicleControl::HasFocus(Viewport && Viewport->Viewport && Viewport->Viewport->HasFocus()))) StopVehicleInput();
		if (!bUnderwaterMovementActive && WantsStationaryTurn() && GetVelocity().Size() < 10.0f)
		{
			PivotInPlace(ManualSteering * StationaryTurnRate * FMath::Min(Delta, 0.1f));
		}
	}
	if (ManualForwardSpeedLimit > 0.0f || ManualReverseSpeedLimit > 0.0f) ApplyManualInput();
	if (bUnderwaterMovementActive) TickUnderwaterMovement(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// Observation keeps its direction by default. Optional auto-centering waits for idle input.
	if (bAutoCenterCamera && GetWorld()->GetTimeSeconds() - LastCameraInputTime >= FMath::Max(CameraAutoCenterDelay, 0.0f))
	{
		SetObservationRotation(FMath::RInterpTo(GetActiveCameraArm()->GetRelativeRotation(),
			GetCameraHomeRotation(), Delta, FMath::Max(CameraAutoCenterSpeed, 0.1f)));
	}
}

void AMqttVehiclePawn::Steering(const FInputActionValue& Value)
{
	// route the input
	DoSteering(Value.Get<float>());
}

void AMqttVehiclePawn::Throttle(const FInputActionValue& Value)
{
	// route the input
	DoThrottle(Value.Get<float>());
}

void AMqttVehiclePawn::Brake(const FInputActionValue& Value)
{
	// route the input
	DoBrake(Value.Get<float>());
}

void AMqttVehiclePawn::StartBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStart();
}

void AMqttVehiclePawn::StopBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStop();
}

void AMqttVehiclePawn::StartHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStart();
}

void AMqttVehiclePawn::StopHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStop();
}

void AMqttVehiclePawn::LookAround(const FInputActionValue& Value)
{
	// route the input
	DoLookAround(Value.Get<float>());
}

void AMqttVehiclePawn::LookUp(const FInputActionValue& Value)
{
	DoLookUp(Value.Get<float>());
}

void AMqttVehiclePawn::RecenterCamera(const FInputActionValue& Value)
{
	DoRecenterCamera();
}

void AMqttVehiclePawn::ToggleCamera(const FInputActionValue& Value)
{
	// route the input
	DoToggleCamera();
}

void AMqttVehiclePawn::ResetVehicle(const FInputActionValue& Value)
{
	// route the input
	DoResetVehicle();
}

void AMqttVehiclePawn::DoSteering(float SteeringValue)
{
	const bool bFreshSteering = FMath::Abs(ManualSteering) < 0.1f;
	ManualSteering = FMath::IsFinite(SteeringValue) ? FMath::Clamp(SteeringValue, -1.0f, 1.0f) : 0.0f;
	if (HandleExplorationManualInput()) return;
	if (bAllowStationaryTurn && bFreshSteering && FMath::Abs(ManualSteering) >= 0.1f) bForcedStop = false;
	ApplyManualInput();
}

void AMqttVehiclePawn::DoThrottle(float ThrottleValue)
{
	ManualThrottle = FMath::IsFinite(ThrottleValue) ? FMath::Clamp(ThrottleValue, -1.0f, 1.0f) : 0.0f;
	if (HandleExplorationManualInput()) return;
	if (!FMath::IsNearlyZero(ManualThrottle))
	{
		bForcedStop = false;
	}
	ApplyManualInput();
}

void AMqttVehiclePawn::DoBrake(float BrakeValue)
{
	ManualBrake = FMath::IsFinite(BrakeValue) ? FMath::Clamp(BrakeValue, 0.0f, 1.0f) : 0.0f;
	if (HandleExplorationManualInput()) return;
	if (ManualBrake > KINDA_SMALL_NUMBER)
	{
		bForcedStop = false;
	}
	ApplyManualInput();
}

void AMqttVehiclePawn::DoBrakeStart()
{
	// call the Blueprint hook for the brake lights
	BrakeLights(true);
}

void AMqttVehiclePawn::DoBrakeStop()
{
	// call the Blueprint hook for the brake lights
	ManualBrake = 0.0f;
	if (HandleExplorationManualInput()) return;
	ApplyManualInput();
	BrakeLights(bManualHandbrake);
}

void AMqttVehiclePawn::DoHandbrakeStart()
{
	bManualHandbrake = true;
	if (bUnderwaterMovementActive)
	{
		StopVehicleInput();
		return;
	}
	if (bExplorationControl)
	{
		StopVehicleInput();
		return;
	}
	if (HandleExplorationManualInput()) return;
	ApplyManualInput();

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void AMqttVehiclePawn::DoHandbrakeStop()
{
	bManualHandbrake = false;
	if (HandleExplorationManualInput()) return;
	ApplyManualInput();

	// call the Blueprint hook for the break lights
	BrakeLights(ManualBrake > KINDA_SMALL_NUMBER);
}

void AMqttVehiclePawn::Ascend(const FInputActionValue& Value) { DoAscend(Value.Get<float>()); }
void AMqttVehiclePawn::Descend(const FInputActionValue& Value) { DoDescend(Value.Get<float>()); }

void AMqttVehiclePawn::DoAscend(float Value)
{
	if (!bUnderwaterMovementActive) return;
	ManualAscend = FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.0f, 1.0f) : 0.0f;
	if (HandleExplorationManualInput()) return;
	if (ManualAscend > 0.0f) bForcedStop = false;
}

void AMqttVehiclePawn::DoDescend(float Value)
{
	if (!bUnderwaterMovementActive) return;
	ManualDescend = FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.0f, 1.0f) : 0.0f;
	if (HandleExplorationManualInput()) return;
	if (ManualDescend > 0.0f) bForcedStop = false;
}

FVector AMqttVehiclePawn::GetVelocity() const
{
	return bUnderwaterMovementActive ? UnderwaterVelocity : Super::GetVelocity();
}

float AMqttVehiclePawn::GetDriveForwardSpeed() const
{
	return bUnderwaterMovementActive ? FVector::DotProduct(UnderwaterVelocity, GetActorForwardVector())
		: ChaosVehicleMovement->GetForwardSpeed();
}

int32 AMqttVehiclePawn::GetDriveGear() const
{
	return bUnderwaterMovementActive ? (GetDriveForwardSpeed() < -1.0f ? -1 : 1) : ChaosVehicleMovement->GetCurrentGear();
}

FBox AMqttVehiclePawn::GetUnderwaterCollisionBox() const
{
	const FTransform Local = SubmersibleHull->GetComponentTransform().GetRelativeTransform(GetActorTransform());
	return SubmersibleHull->GetStaticMesh()->GetBoundingBox().TransformBy(Local).ExpandBy(2.0);
}

bool AMqttVehiclePawn::IsUnderwaterTurnClear(float DeltaDegrees) const
{
	if (!FMath::IsFinite(DeltaDegrees) || FMath::Abs(DeltaDegrees) > 180.0f) return false;
	const FBox Box = GetUnderwaterCollisionBox();
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(UnderwaterTurn), false, this);
	const int32 Steps = FMath::Max(1, FMath::CeilToInt(FMath::Abs(DeltaDegrees) / 2.0f));
	for (int32 Index = 0; Index <= Steps; ++Index)
	{
		const FQuat Rotation = FRotator(0, GetActorRotation().Yaw + DeltaDegrees * Index / Steps, 0).Quaternion();
		if (GetWorld()->OverlapBlockingTestByChannel(GetActorLocation() + Rotation.RotateVector(Box.GetCenter()),
			Rotation, ECC_Vehicle, FCollisionShape::MakeBox(Box.GetExtent()), Params)) return false;
	}
	return true;
}

void AMqttVehiclePawn::HaltUnderwaterMovement()
{
	AutomaticMotorInput = FVector2D::ZeroVector;
	AutomaticMotorInputAt = -1.0;
	UnderwaterVelocity = FVector::ZeroVector;
	bUnderwaterReturnTarget = false;
}

bool AMqttVehiclePawn::IsUnderwaterPathClear(const FVector& Target) const
{
	if (!bUnderwaterMovementActive || Target.ContainsNaN()) return false;
	const FVector Offset = Target - GetActorLocation();
	const FQuat Rotation = Offset.SizeSquared2D() > 25.0
		? FRotator(0, Offset.Rotation().Yaw, 0).Quaternion() : GetActorQuat();
	const FBox Box = GetUnderwaterCollisionBox();
	const FVector Centre = GetActorLocation() + Rotation.RotateVector(Box.GetCenter());
	FHitResult Hit;
	return !GetWorld()->SweepSingleByChannel(Hit, Centre, Centre + Offset, Rotation, ECC_Vehicle,
		FCollisionShape::MakeBox(Box.GetExtent()), FCollisionQueryParams(SCENE_QUERY_STAT(UnderwaterReturnLookahead), false, this));
}

FBox AMqttVehiclePawn::GetUnderwaterNavigationEnvelope() const
{
 // An axis-aligned envelope encloses the entire hull at every yaw, including its offset.
 // Keep the actual vertical bounds so a valid low hover is not inflated into the floor.
 const FBox Hull = GetUnderwaterCollisionBox();
 const double X = FMath::Max(FMath::Abs(Hull.Min.X), FMath::Abs(Hull.Max.X));
 const double Y = FMath::Max(FMath::Abs(Hull.Min.Y), FMath::Abs(Hull.Max.Y));
 const double Radius = FMath::Sqrt(X*X + Y*Y) + 5.0;
 return FBox(FVector(-Radius,-Radius,Hull.Min.Z), FVector(Radius,Radius,Hull.Max.Z));
}

bool AMqttVehiclePawn::IsUnderwaterNavigationSegmentClear(const FVector& From, const FVector& To) const
{
 if(!bUnderwaterMovementActive || !GetWorld() || From.ContainsNaN() || To.ContainsNaN()
  || From.Z > UnderwaterCeilingZ+0.01 || To.Z > UnderwaterCeilingZ+0.01) return false;
 const FBox Box = GetUnderwaterNavigationEnvelope();
 const FCollisionQueryParams Params(SCENE_QUERY_STAT(UnderwaterNavigation), false, this);
 const FCollisionShape Shape = FCollisionShape::MakeBox(Box.GetExtent());
 const FVector A = From + Box.GetCenter(), B = To + Box.GetCenter();
 if(GetWorld()->OverlapBlockingTestByChannel(A,FQuat::Identity,ECC_Vehicle,Shape,Params)
  || GetWorld()->OverlapBlockingTestByChannel(B,FQuat::Identity,ECC_Vehicle,Shape,Params)) return false;
 FHitResult Hit;
 return !GetWorld()->SweepSingleByChannel(Hit,A,B,FQuat::Identity,ECC_Vehicle,Shape,Params);
}

void AMqttVehiclePawn::ApplyUnderwaterReturnTarget(const FVector& Target, float Speed)
{
	if (!bUnderwaterMovementActive || !bExplorationControl) return;
	if (Target.ContainsNaN() || !FMath::IsFinite(Speed) || Speed <= 0.0f)
	{
		StopVehicleInput();
		return;
	}
	UnderwaterReturnTarget = Target;
	UnderwaterReturnSpeed = FMath::Min(Speed, GetReturnCruiseSpeed());
	bUnderwaterReturnTarget = true;
}

void AMqttVehiclePawn::TickUnderwaterMovement(float Delta)
{
	if (!FMath::IsFinite(Delta) || Delta <= 0.0f) return;
	const float StepTime = FMath::Min(Delta, 0.1f);
	if (bExplorationControl && bUnderwaterReturnTarget)
	{
		AutomaticMotorInput = FVector2D::ZeroVector;
		AutomaticMotorInputAt = GetWorld()->GetTimeSeconds();
	}
	const float VerticalLimit = FMath::IsFinite(VerticalSwimSpeed) ? FMath::Clamp(VerticalSwimSpeed, 10.0f, 200.0f) : 100.0f;
	FVector Desired = FVector::ZeroVector;
	float YawStep = 0.0f;
	if (bExplorationControl && bUnderwaterReturnTarget)
	{
		const FVector Offset = UnderwaterReturnTarget - GetActorLocation();
		if (Offset.SizeSquared2D() > 25.0)
		{
			const float Error = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, Offset.Rotation().Yaw);
			YawStep = FMath::Clamp(Error, -StationaryTurnRate * StepTime, StationaryTurnRate * StepTime);
			// Turn toward the path before translating; never reverse or cut a corner sideways.
			if (FMath::Abs(Error) > 10.0f) Desired = FVector::ZeroVector;
			else Desired = Offset.GetSafeNormal() * FMath::Min(UnderwaterReturnSpeed, static_cast<float>(Offset.Size()) * 2.0f);
		}
		else Desired.Z = FMath::Clamp(static_cast<float>(Offset.Z) * 2.0f, -VerticalLimit, VerticalLimit);
		if (FMath::Abs(Desired.Z) > VerticalLimit) Desired *= VerticalLimit / FMath::Abs(Desired.Z);
	}
	else if (!bExplorationControl && !bRequireManualRelease && !bForcedStop && !bManualHandbrake)
	{
		const float Forward = FMath::Max(ManualThrottle, 0.0f);
		const float Reverse = FMath::Max(-ManualThrottle, ManualBrake);
		if (!(Forward > 0.0f && Reverse > 0.0f))
		{
			YawStep = ManualSteering * StationaryTurnRate * StepTime;
			const float ForwardLimit = ManualForwardSpeedLimit > 0 ? ManualForwardSpeedLimit / 0.036f : 6 * 44.704f;
			const float ReverseLimit = ManualReverseSpeedLimit > 0 ? ManualReverseSpeedLimit / 0.036f : 3 / 0.036f;
			const float Speed = Forward * ForwardLimit - Reverse * ReverseLimit;
			Desired = FRotator(0, GetActorRotation().Yaw + YawStep, 0).Vector() * Speed;
			Desired.Z = (ManualAscend - ManualDescend) * VerticalLimit;
			Desired = Desired.GetClampedToMaxSize(ForwardLimit);
		}
	}
	if (!FMath::IsNearlyZero(YawStep))
	{
		if (!IsUnderwaterTurnClear(YawStep)) { UnderwaterVelocity = FVector::ZeroVector; return; }
		SetActorRotation(FRotator(0, GetActorRotation().Yaw + YawStep, 0));
		if (bExplorationControl && bUnderwaterReturnTarget)
			AutomaticMotorInput.Y = FMath::Clamp(YawStep / FMath::Max(StationaryTurnRate * StepTime, 0.001f), -1.0f, 1.0f);
	}
	// Releasing a direction holds that axis immediately; propulsion ramps up smoothly.
	for (int32 Axis = 0; Axis < 3; ++Axis)
		UnderwaterVelocity[Axis] = FMath::IsNearlyZero(Desired[Axis]) ? 0.0
			: FMath::FInterpConstantTo(UnderwaterVelocity[Axis], Desired[Axis], StepTime, 180.0);
	const FVector Start = GetActorLocation();
	FVector Travel = UnderwaterVelocity * StepTime;
	if (Start.Z + Travel.Z > UnderwaterCeilingZ) Travel.Z = FMath::Max(0.0, UnderwaterCeilingZ - Start.Z);
	if (Travel.IsNearlyZero()) { UnderwaterVelocity = FVector::ZeroVector; return; }
	const FBox Box = GetUnderwaterCollisionBox();
	const FQuat Rotation = GetActorQuat();
	const FVector Centre = Start + Rotation.RotateVector(Box.GetCenter());
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(UnderwaterTravel), false, this);
	const bool bBlocked = GetWorld()->SweepSingleByChannel(Hit, Centre, Centre + Travel, Rotation,
		ECC_Vehicle, FCollisionShape::MakeBox(Box.GetExtent()), Params);
	if (bBlocked) Travel *= FMath::Max(0.0, static_cast<double>(Hit.Time) - 1.0 / FMath::Max(Travel.Size(), 1.0));
	SetActorLocation(Start + Travel, false, nullptr, ETeleportType::TeleportPhysics);
	UnderwaterVelocity = bBlocked ? FVector::ZeroVector : Travel / StepTime;
	if (bExplorationControl && bUnderwaterReturnTarget)
	{
		// A blocked virtual hull must not keep commanding the real car forward.
		AutomaticMotorInput.X = bBlocked ? 0.0f : FMath::Clamp(
			FVector::DotProduct(UnderwaterVelocity, GetActorForwardVector()) / GetReturnCruiseSpeed(), 0.0f, 1.0f);
	}
}

void AMqttVehiclePawn::ApplyManualInput()
{
	if (bExplorationControl || bRequireManualRelease) return;
	if (bUnderwaterMovementActive)
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
		ChaosVehicleMovement->SetBrakeInput(0.0f);
		ChaosVehicleMovement->SetSteeringInput(0.0f);
		ChaosVehicleMovement->SetHandbrakeInput(true);
		return;
	}
	// Match S-to-brake/reverse when changing back to forward travel as well.
	ChaosVehicleMovement->bThrottleAsBrake = true;
	const float Forward = FMath::Max(ManualThrottle, 0.0f);
	const float Reverse = FMath::Max(-ManualThrottle, ManualBrake);
	// The template's brake pedal also reverses through Chaos bReverseAsBrake.
	// Holding both pedals must stop regardless of input callback order.
	const bool bStop = bForcedStop || bManualHandbrake || WantsStationaryTurn() || (Forward > 0.0f && Reverse > 0.0f);
	const float Speed = ChaosVehicleMovement->GetForwardSpeed();
	const float Limit = (Speed >= 0.0f ? ManualForwardSpeedLimit : ManualReverseSpeedLimit) / 0.036f;
	const bool bOverspeed = Limit > 0.0f && FMath::Abs(Speed) > Limit + 5.0f;
	const float Taper = Limit > 0.0f ? FMath::Clamp((Limit - FMath::Abs(Speed)) / 30.0f, 0.0f, 1.0f) : 1.0f;
	// Only taper a pedal propelling the current direction; keep opposite-pedal braking intact.
	ChaosVehicleMovement->SetThrottleInput(bStop || bOverspeed ? 0.0f : Forward * (Speed >= 0.0f ? Taper : 1.0f));
	ChaosVehicleMovement->SetBrakeInput(bStop || bOverspeed ? 0.0f : Reverse * (Speed <= 0.0f ? Taper : 1.0f));
	ChaosVehicleMovement->SetSteeringInput(bStop ? 0.0f : ManualSteering);
	ChaosVehicleMovement->SetHandbrakeInput(bStop || bOverspeed);
}

bool AMqttVehiclePawn::WantsStationaryTurn() const
{
	return bAllowStationaryTurn && !bExplorationControl && !bRequireManualRelease && !bForcedStop
		&& !bManualHandbrake && FMath::Abs(ManualThrottle) < 0.1f && ManualBrake < 0.1f
		&& FMath::Abs(ManualSteering) >= 0.1f;
}

float AMqttVehiclePawn::GetReturnCruiseSpeed() const
{
	const float Cruise = FMath::IsFinite(AutomaticCruiseSpeedMph)
		? FMath::Clamp(AutomaticCruiseSpeedMph, 1.0f, 6.0f) * 44.704f : 5.0f * 44.704f;
	return FMath::IsFinite(ManualForwardSpeedLimit) && ManualForwardSpeedLimit > 0.0f
		? FMath::Min(Cruise, ManualForwardSpeedLimit / 0.036f) : Cruise;
}

bool AMqttVehiclePawn::PivotInPlace(float DeltaDegrees)
{
	if (!bAllowStationaryTurn || !FMath::IsFinite(DeltaDegrees) || FMath::Abs(DeltaDegrees) > 10.0f
		|| GetVelocity().Size() >= 10.0f || FVector::DotProduct(GetActorUpVector(), FVector::UpVector) < 0.9) return false;
	const auto Arc = ExplorationRoute::PivotTurn(GetActorLocation(), GetActorForwardVector(), DeltaDegrees);
	if (!ExplorationRoute::IsTurnClear(*GetWorld(), Arc, this)) return false;
	FRotator Rotation = GetActorRotation();
	Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + DeltaDegrees);
	if (bUnderwaterMovementActive)
	{
		HaltUnderwaterMovement();
		const bool bRotated = SetActorRotation(Rotation);
		if (bRotated && bExplorationControl)
		{
			AutomaticMotorInput = FVector2D(0, FMath::Sign(DeltaDegrees));
			AutomaticMotorInputAt = GetWorld()->GetTimeSeconds();
		}
		return bRotated;
	}
	// Chaos supplies translation; the stationary yaw is an explicitly local simulation control.
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	return SetActorRotation(Rotation, ETeleportType::TeleportPhysics);
}

bool AMqttVehiclePawn::CanSelectSubmersible() const
{
	if (!HasSubmersibleChoices() || bExplorationControl || GetVelocity().Size() >= 10.0f || !AreManualControlsNeutral()) return false;
	TArray<AActor*> Homes;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("ExplorationReturnAnchor"), Homes);
	return Homes.Num() == 1 && FVector::Dist2D(GetActorLocation(), Homes[0]->GetActorLocation()) <= 500.0
		&& FMath::Abs(GetActorLocation().Z - Homes[0]->GetActorLocation().Z) <= 200.0;
}

bool AMqttVehiclePawn::SelectSubmersible(int32 Variant)
{
	if (Variant != 1 || !CanSelectSubmersible()) return false;
	StopVehicleInput();
	SubmersibleVariant = Variant;
	SubmersibleHull->SetStaticMesh(LanzamanSubmersible);
	FrontSpringArm->SetRelativeLocation(FVector(-90,-62,320));
	SubmersibleView->RefreshInterior();
	return true;
}

FVector2D AMqttVehiclePawn::GetAutomaticVehicleControlInput() const
{
	if (!bUnderwaterMovementActive || !bExplorationControl || bRequireManualRelease || !GetWorld()
		|| AutomaticMotorInputAt < 0.0 || GetWorld()->GetTimeSeconds() - AutomaticMotorInputAt > 0.2)
		return FVector2D::ZeroVector;
	return AutomaticMotorInput;
}

FVector2D AMqttVehiclePawn::GetVehicleControlInput() const
{
	// Real RC path return has not been validated. Keep all autonomous/held output at zero.
	if (bExplorationControl || bRequireManualRelease) return FVector2D::ZeroVector;
	const float Forward = FMath::Max(ManualThrottle, 0.0f);
	const float Reverse = FMath::Max(-ManualThrottle, ManualBrake);
	if (bForcedStop || bManualHandbrake || (Forward > 0.0f && Reverse > 0.0f))
	{
		return FVector2D::ZeroVector;
	}

	// The submerged hull is kinematic: its Chaos gear/speed is not a pedal state.
	// Horizontal manual input includes differential-drive pivots. Vertical motion
	// remains local to Unreal; the bridge still requires explicit opt-in.
	if (bUnderwaterMovementActive)
	{
		const float Throttle = Forward - Reverse;
		return FVector2D(Throttle, ManualSteering);
	}

	const float Speed = ChaosVehicleMovement->GetForwardSpeed();
	// Do not command opposite motor travel until the virtual vehicle has slowed
	// to within 1 cm/s of rest; Chaos's internal braking threshold is protected.
	constexpr float Threshold = 1.0f;
	float Throttle = 0.0f;
	if (Forward > 0.0f)
	{
		const bool bBraking = ChaosVehicleMovement->bReverseAsBrake
			&& ChaosVehicleMovement->bThrottleAsBrake && Speed < -Threshold;
		Throttle = (bBraking || ChaosVehicleMovement->GetTargetGear() < 0) ? 0.0f : Forward;
	}
	else if (Reverse > 0.0f && ChaosVehicleMovement->bReverseAsBrake
		&& Speed <= Threshold && ChaosVehicleMovement->GetTargetGear() < 0)
	{
		Throttle = -Reverse;
	}

	// A stopped vehicle must not make a differential-drive receiver pivot in place.
	return FVector2D(Throttle, FMath::IsNearlyZero(Throttle) ? 0.0f : ManualSteering);
}

void AMqttVehiclePawn::StopVehicleInput()
{
	HaltUnderwaterMovement();
	if (bExplorationControl)
	{
		EndExplorationControl(true);
		OnExplorationControlInterrupted.Broadcast(false);
		return;
	}
	if (bRequireManualRelease || (bAllowStationaryTurn && !AreManualControlsNeutral()))
	{
		EndExplorationControl(true);
		return;
	}
	ManualThrottle = ManualBrake = ManualSteering = ManualAscend = ManualDescend = 0.0f;
	bManualHandbrake = false;
	bForcedStop = true;
	ApplyManualInput();
}

bool AMqttVehiclePawn::AreManualControlsNeutral() const
{
	return FMath::Abs(ManualThrottle) < 0.1f && ManualBrake < 0.1f
		&& FMath::Abs(ManualSteering) < 0.1f && ManualAscend < 0.1f && ManualDescend < 0.1f && !bManualHandbrake;
}

bool AMqttVehiclePawn::HandleExplorationManualInput()
{
	if (!bExplorationControl && !bRequireManualRelease) return false;
	if (AreManualControlsNeutral())
	{
		bManualOverrideArmed = true;
		bRequireManualRelease = false;
	}
	else if (bExplorationControl && bManualOverrideArmed)
	{
		// Stop first; subsequent fresh input is handled by the normal manual path.
		EndExplorationControl(false);
		OnExplorationControlInterrupted.Broadcast(true);
	}
	return true;
}

bool AMqttVehiclePawn::IsManualDriveRequested() const
{
	return !bExplorationControl && !bRequireManualRelease && !bForcedStop
		&& (FMath::Abs(ManualThrottle) > 0.1f || ManualBrake > 0.1f || WantsStationaryTurn()
			|| ManualAscend > 0.1f || ManualDescend > 0.1f);
}

void AMqttVehiclePawn::BeginExplorationControl()
{
	if (bExplorationControl) return;
	bSavedReverseAsBrake = ChaosVehicleMovement->bReverseAsBrake;
	bSavedThrottleAsBrake = ChaosVehicleMovement->bThrottleAsBrake;
	bSavedAutomaticGears = ChaosVehicleMovement->GetUseAutoGears();
	bExplorationControl = true;
	bRequireManualRelease = false;
	bManualOverrideArmed = AreManualControlsNeutral();
	ChaosVehicleMovement->bReverseAsBrake = false;
	ChaosVehicleMovement->bThrottleAsBrake = false;
	ChaosVehicleMovement->SetUseAutomaticGears(false);
	ApplyExplorationControl(0.0f, 0.0f, 1.0f, true);
}

void AMqttVehiclePawn::ApplyExplorationControl(float SignedThrottle, float Steering, float Brake, bool bHold)
{
	if (!bExplorationControl) return;
	if (bUnderwaterMovementActive)
	{
		// The return component supplies a 3D target for swimming; this API holds during preparation/turns.
		HaltUnderwaterMovement();
		return;
	}
	if (!FMath::IsFinite(SignedThrottle) || !FMath::IsFinite(Steering) || !FMath::IsFinite(Brake))
	{
		StopVehicleInput();
		return;
	}
	SignedThrottle = FMath::Clamp(SignedThrottle, -0.65f, 0.65f);
	Brake = FMath::Clamp(Brake, 0.0f, 1.0f);
	if (ChaosVehicleMovement->GetForwardSpeed() > GetReturnCruiseSpeed())
	{
		SignedThrottle = 0.0f;
		Brake = FMath::Max(Brake, 0.1f);
	}
	if (!FMath::IsNearlyZero(SignedThrottle))
	{
		const int32 Gear = SignedThrottle > 0.0f ? 1 : -1;
		if (ChaosVehicleMovement->GetTargetGear() != Gear)
		{
			if (FMath::Abs(ChaosVehicleMovement->GetForwardSpeed()) > 20.0f)
			{
				SignedThrottle = 0.0f;
				Brake = 1.0f;
			}
			else ChaosVehicleMovement->SetTargetGear(Gear, true);
		}
	}
	ChaosVehicleMovement->SetThrottleInput(bHold || Brake > 0.0f ? 0.0f : FMath::Abs(SignedThrottle));
	ChaosVehicleMovement->SetBrakeInput(bHold ? 1.0f : Brake);
	ChaosVehicleMovement->SetSteeringInput(bHold ? 0.0f : FMath::Clamp(Steering, -1.0f, 1.0f));
	ChaosVehicleMovement->SetHandbrakeInput(bHold);
	BrakeLights(bHold || Brake > 0.0f);
}

void AMqttVehiclePawn::EndExplorationControl(bool bRequireRelease)
{
	HaltUnderwaterMovement();
	if (bExplorationControl)
	{
		ChaosVehicleMovement->bReverseAsBrake = bSavedReverseAsBrake;
		ChaosVehicleMovement->bThrottleAsBrake = bSavedThrottleAsBrake;
		ChaosVehicleMovement->SetUseAutomaticGears(bSavedAutomaticGears);
	}
	bExplorationControl = false;
	bRequireManualRelease = bRequireRelease && !AreManualControlsNeutral();
	if (!bRequireRelease)
	{
		ManualThrottle = ManualBrake = ManualSteering = ManualAscend = ManualDescend = 0.0f;
		bManualHandbrake = false;
	}
	// Preserve held values only to require a full release after a stop, never to drive.
	bForcedStop = true;
	ChaosVehicleMovement->SetThrottleInput(0.0f);
	ChaosVehicleMovement->SetBrakeInput(0.0f);
	ChaosVehicleMovement->SetSteeringInput(0.0f);
	ChaosVehicleMovement->SetHandbrakeInput(true);
	BrakeLights(true);
}

void AMqttVehiclePawn::DoLookAround(float YawDelta)
{
	if (!IsCameraInputAllowed() || !FMath::IsFinite(YawDelta) || FMath::IsNearlyZero(YawDelta))
	{
		return;
	}
	FRotator Rotation = GetActiveCameraArm()->GetRelativeRotation();
	Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + YawDelta);
	SetObservationRotation(Rotation);
	MarkCameraInput();
}

void AMqttVehiclePawn::DoLookUp(float PitchDelta)
{
	if (!IsCameraInputAllowed() || !FMath::IsFinite(PitchDelta) || FMath::IsNearlyZero(PitchDelta))
	{
		return;
	}
	FRotator Rotation = GetActiveCameraArm()->GetRelativeRotation();
	Rotation.Pitch += bInvertCameraPitch ? -PitchDelta : PitchDelta;
	SetObservationRotation(Rotation);
	MarkCameraInput();
}

void AMqttVehiclePawn::DoRecenterCamera()
{
	if (IsCameraInputAllowed())
	{
		SetObservationRotation(GetCameraHomeRotation());
		if (bVRCockpitInitialized && UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition())
		{
			UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
		}
		MarkCameraInput();
	}
}

void AMqttVehiclePawn::InitializeVRCockpit()
{
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (bVRCockpitInitialized || !HasSubmersibleChoices() || !PC || !PC->IsLocalController()
		|| !UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()
		|| !UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition())
	{
		return;
	}

	// The existing front boom is already the seated eye point. A floor/room
	// origin adds the real user's height and lateral offset on top of that seat.
	// Local space lets OpenXR recenter all three position axes at the first
	// valid tracked pose; subsequent head motion and late update remain active.
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Local);
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->SocketOffset = FVector::ZeroVector;
	FrontSpringArm->TargetOffset = FVector::ZeroVector;
	FrontCamera->bUsePawnControlRotation = false;
	BackCamera->bUsePawnControlRotation = false;
	FrontCamera->bLockToHmd = BackCamera->bLockToHmd = true;
	bVRCockpitInitialized = true;
	UE_LOG(LogMqttVehicle, Log, TEXT("VR cockpit: seated Local tracking origin centered; R recenters the headset."));
}

USpringArmComponent* AMqttVehiclePawn::GetActiveCameraArm() const
{
	return bFrontCameraActive ? FrontSpringArm : BackSpringArm;
}

FRotator AMqttVehiclePawn::GetCameraHomeRotation() const
{
	return bFrontCameraActive ? FrontCameraHomeRotation : BackCameraHomeRotation;
}

void AMqttVehiclePawn::SetObservationRotation(const FRotator& Rotation)
{
	const float MinPitch = FMath::Clamp(FMath::Min(CameraMinPitch, CameraMaxPitch), -89.0f, 89.0f);
	const float MaxPitch = FMath::Clamp(FMath::Max(CameraMinPitch, CameraMaxPitch), -89.0f, 89.0f);
	const FRotator SharedRotation(FMath::Clamp(Rotation.Pitch, MinPitch, MaxPitch),
		FRotator::NormalizeAxis(Rotation.Yaw), 0.0f);
	GetActiveCameraArm()->SetRelativeRotation(SharedRotation);
	if (HasSubmersibleChoices())
	{
		FrontSpringArm->SetRelativeRotation(SharedRotation);
		BackSpringArm->SetRelativeRotation(SharedRotation);
	}
}

bool AMqttVehiclePawn::IsCameraInputAllowed() const
{
	const APlayerController* PC = Cast<APlayerController>(GetController());
	return !PC || !PC->IsLookInputIgnored();
}

void AMqttVehiclePawn::MarkCameraInput()
{
	LastCameraInputTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void AMqttVehiclePawn::DoToggleCamera()
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
	MarkCameraInput();
}

void AMqttVehiclePawn::DoResetVehicle()
{
	++ExplorationResetRevision;
	StopVehicleInput();
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void AMqttVehiclePawn::FlippedCheck()
{
	// check the difference in angle between the mesh's up vector and world up
	const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());

	if (UpDot < FlipCheckMinDot)
	{
		// is this the second time we've checked that the vehicle is still flipped?
		if (bPreviousFlipCheck)
		{
			// reset the vehicle to upright
			DoResetVehicle();
		}
		
		// set the flipped check flag so the next check resets the car
		bPreviousFlipCheck = true;

	} else {

		// we're upright. reset the flipped check flag
		bPreviousFlipCheck = false;
	}
}

#undef LOCTEXT_NAMESPACE
