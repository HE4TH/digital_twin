// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "MqttVehiclePawn.generated.h"

class UCameraComponent;
class USubmersibleViewComponent;
class USubmersiblePropulsionComponent;
class UStaticMeshComponent;
class UStaticMesh;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE_OneParam(FExplorationControlInterrupted, bool /* bManualOverride */);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class AMqttVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetVehicleAction;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

	/** Keep the observation direction until the player explicitly recenters. */
	UPROPERTY(EditAnywhere, Category="Camera|Observation")
	bool bAutoCenterCamera = false;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="0.0", Units="s"))
	float CameraAutoCenterDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="0.1"))
	float CameraAutoCenterSpeed = 2.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="-89.0", ClampMax="89.0", Units="deg"))
	float CameraMinPitch = -45.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="-89.0", ClampMax="89.0", Units="deg"))
	float CameraMaxPitch = 70.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation")
	bool bInvertCameraPitch = false;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="0.01"))
	float MousePitchSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation", meta=(ClampMin="1.0"))
	float GamepadPitchDegreesPerSecond = 60.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Observation")
	bool bUseObservationCameraCollision = true;

	/** Keeps track of whether the car is flipped. If this is true for two flip checks, resets the vehicle automatically */
	bool bPreviousFlipCheck = false;

	/** Time between automatic flip checks */
	UPROPERTY(EditAnywhere, Category="Flip Check", meta = (Units = "s"))
	float FlipCheckTime = 3.0f;

	/** Minimum dot product value for the vehicle's up direction that we still consider upright */
	UPROPERTY(EditAnywhere, Category="Flip Check")
	float FlipCheckMinDot = -0.2f;

	/** Flip check timer */
	FTimerHandle FlipCheckTimer;

public:
	AMqttVehiclePawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Appearance")
	TObjectPtr<UStaticMeshComponent> SubmersibleHull;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Appearance")
	TObjectPtr<UStaticMesh> IrisSubmersible;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Appearance")
	TObjectPtr<UStaticMesh> LanzamanSubmersible;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Control", meta=(ClampMin="0", Units="km/h"))
	float ManualForwardSpeedLimit = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Control", meta=(ClampMin="0", Units="km/h"))
	float ManualReverseSpeedLimit = 0.0f;
	/** Independent Unreal return cruise target; manual forward limit remains an upper bound. */
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Control", meta=(ClampMin="1", ClampMax="6", Units="mph"))
	float AutomaticCruiseSpeedMph = 5.0f;
	/** Local underwater simulation for the exploration hull, independent of RC control. */
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Underwater")
	bool bEnableUnderwaterMovement = true;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Underwater", meta=(ClampMin="10", ClampMax="200", Units="cm/s"))
	float VerticalSwimSpeed = 100.0f;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Underwater", meta=(ClampMin="100", Units="cm"))
	float MaximumRiseAboveStart = 3000.0f;
	bool IsUnderwaterMovementEnabled() const { return bUnderwaterMovementActive; }
	virtual FVector GetVelocity() const override;
	float GetDriveForwardSpeed() const;
	int32 GetDriveGear() const;
	bool IsUnderwaterTurnClear(float DeltaDegrees) const;
	bool IsUnderwaterPathClear(const FVector& Target) const;
	FBox GetUnderwaterNavigationEnvelope() const;
	bool IsUnderwaterNavigationSegmentClear(const FVector& From, const FVector& To) const;
	double GetUnderwaterCeiling() const { return UnderwaterCeilingZ; }
	void ApplyUnderwaterReturnTarget(const FVector& Target, float Speed);
	bool SelectSubmersible(int32 Variant);
	bool CanSelectSubmersible() const;
	int32 GetSubmersibleVariant() const { return SubmersibleVariant; }
	// Legacy API name retained for existing assets; Iris is no longer a playable choice.
	bool HasSubmersibleChoices() const { return LanzamanSubmersible != nullptr; }
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Appearance", meta=(ClampMin="0", ClampMax="1"))
	int32 DefaultSubmersibleVariant = 1;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Control")
	bool bAllowStationaryTurn = false;
	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Control", meta=(ClampMin="10", ClampMax="90", Units="deg/s"))
	float StationaryTurnRate = 45.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USubmersibleViewComponent> SubmersibleView;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Effects")
	TObjectPtr<USubmersiblePropulsionComponent> PropulsionEffects;
	bool IsInteriorCameraActive() const { return bFrontCameraActive; }
	/** Rotate only the local simulated hull, checking its swept footprint first. */
	bool PivotInPlace(float DeltaDegrees);
	float GetReturnCruiseSpeed() const;

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void UnPossessed() override;

	// End Pawn interface

	// Begin Actor interface

	/** Initialization */
	virtual void BeginPlay() override;

	/** Cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Update */
	virtual void Tick(float Delta) override;

	// End Actor interface

protected:

	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);
	void LookUp(const FInputActionValue& Value);
	void RecenterCamera(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

public:

	/** Handle steering input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSteering(float SteeringValue);

	/** Handle throttle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrottle(float ThrottleValue);

	/** Handle brake input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrake(float BrakeValue);

	/** Handle brake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStart();

	/** Handle brake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStop();
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoAscend(float Value);
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoDescend(float Value);

	/** Handle handbrake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStart();

	/** Handle handbrake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStop();

	/** Handle look input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoLookAround(float YawDelta);

	/** Pitch the active observation camera by degrees; positive looks upward. */
	UFUNCTION(BlueprintCallable, Category="Camera|Observation")
	void DoLookUp(float PitchDelta);

	/** Restore only the active camera's initial orientation without moving the vehicle. */
	UFUNCTION(BlueprintCallable, Category="Camera|Observation")
	void DoRecenterCamera();

	/** Handle toggle camera input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleCamera();

	/** Handle reset vehicle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoResetVehicle();

	/** Signed drive command (X: forward/reverse, Y: steering). Braking produces zero output. */
	UFUNCTION(BlueprintPure, Category="Vehicle|Control")
	FVector2D GetVehicleControlInput() const;
	/** Observed virtual horizontal return motion, not physical position feedback. */
	FVector2D GetAutomaticVehicleControlInput() const;

	/** Clear held inputs and hold the vehicle still until fresh drive input arrives. */
	UFUNCTION(BlueprintCallable, Category="Vehicle|Control")
	void StopVehicleInput();

	/** Local simulation control. These commands are never returned by GetVehicleControlInput. */
	void BeginExplorationControl();
	void ApplyExplorationControl(float SignedThrottle, float Steering, float Brake, bool bHold);
	void EndExplorationControl(bool bRequireRelease);
	bool IsExplorationControlActive() const { return bExplorationControl; }
	bool IsManualDriveRequested() const;
	uint32 GetExplorationResetRevision() const { return ExplorationResetRevision; }
	FExplorationControlInterrupted OnExplorationControlInterrupted;

private:
	int32 SubmersibleVariant = INDEX_NONE;
	void SetupObservationInput(UEnhancedInputComponent* Input);
	void RemoveObservationInput();
	USpringArmComponent* GetActiveCameraArm() const;
	FRotator GetCameraHomeRotation() const;
	void SetObservationRotation(const FRotator& Rotation);
	bool IsCameraInputAllowed() const;
	void MarkCameraInput();
	void InitializeVRCockpit();
	bool bVRCockpitInitialized = false;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> ObservationMappingContext;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ObservationPitchAction;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ObservationRecenterAction;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ObservationToggleCameraAction;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> AscendAction;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> DescendAction;
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> ObservationInputSubsystem;
	FRotator FrontCameraHomeRotation = FRotator::ZeroRotator;
	FRotator BackCameraHomeRotation = FRotator::ZeroRotator;
	float LastCameraInputTime = 0.0f;

	void ApplyManualInput();
	bool WantsStationaryTurn() const;
	float ManualThrottle = 0.0f;
	float ManualBrake = 0.0f;
	float ManualSteering = 0.0f;
	float ManualAscend = 0.0f;
	float ManualDescend = 0.0f;
	void Ascend(const FInputActionValue& Value);
	void Descend(const FInputActionValue& Value);
	void TickUnderwaterMovement(float Delta);
	void HaltUnderwaterMovement();
	FBox GetUnderwaterCollisionBox() const;
	bool bUnderwaterMovementActive = false;
	bool bUnderwaterReturnTarget = false;
	FVector2D AutomaticMotorInput = FVector2D::ZeroVector;
	double AutomaticMotorInputAt = -1.0;
	FVector UnderwaterVelocity = FVector::ZeroVector;
	FVector UnderwaterReturnTarget = FVector::ZeroVector;
	float UnderwaterReturnSpeed = 0.0f;
	double UnderwaterCeilingZ = 0.0;
	bool bManualHandbrake = false;
	bool bForcedStop = false;
	bool HandleExplorationManualInput();
	bool AreManualControlsNeutral() const;
	bool bExplorationControl = false;
	bool bManualOverrideArmed = false;
	bool bRequireManualRelease = false;
	bool bSavedReverseAsBrake = true;
	bool bSavedThrottleAsBrake = true;
	bool bSavedAutomaticGears = true;
	uint32 ExplorationResetRevision = 0;

protected:

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

	/** Checks if the car is flipped upside down and automatically resets it */
	UFUNCTION()
	void FlippedCheck();

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }
};
