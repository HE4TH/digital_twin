// Copyright Epic Games, Inc. All Rights Reserved.


#include "MqttVehiclePlayerController.h"
#include "MqttVehiclePawn.h"
#include "SubmersibleViewComponent.h"
#include "Components/PrimitiveComponent.h"
#include "MqttVehicleUI.h"
#include "WhaleObservationComponent.h"
#include "WhaleObservationWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/StereoLayerComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "ExplorationReturnComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "MqttVehicle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Widgets/Input/SVirtualJoystick.h"

AMqttVehiclePlayerController::AMqttVehiclePlayerController()
{
	WhaleObservation = CreateDefaultSubobject<UWhaleObservationComponent>(TEXT("WhaleObservation"));
	ExplorationReturn = CreateDefaultSubobject<UExplorationReturnComponent>(TEXT("ExplorationReturn"));
}

void AMqttVehiclePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// ensure we're attached to the vehicle pawn so that World Partition streaming works correctly
	bAttachToPawn = true;

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogMqttVehicle, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}
		

		// spawn the UI widget and add it to the viewport
		VehicleUI = CreateWidget<UMqttVehicleUI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			// The exploration overlay replaces the template's speed/gear HUD.
			VehicleUI->SetVisibility(IsValid(VehiclePawn) && !VehiclePawn->HasSubmersibleChoices()
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			VehicleUI->AddToViewport();

		} else {

			UE_LOG(LogMqttVehicle, Error, TEXT("Could not spawn vehicle UI widget."));

		}

		WhaleObservationUI = CreateWidget<UWhaleObservationWidget>(this, UWhaleObservationWidget::StaticClass());
		if (WhaleObservationUI)
		{
			WhaleObservation->OnObservationStateChanged.AddDynamic(WhaleObservationUI, &UWhaleObservationWidget::ApplyObservationState);
			WhaleObservationUI->ApplyObservationState(WhaleObservation->GetObservationState());
			ExplorationReturn->OnReturnStateChanged.AddDynamic(WhaleObservationUI, &UWhaleObservationWidget::ApplyReturnState);
			WhaleObservationUI->ApplyReturnState(ExplorationReturn->GetReturnState());
            WhaleObservationUI->bVRPresentation = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
            if (!WhaleObservationUI->bVRPresentation) WhaleObservationUI->AddToPlayerScreen(10);
            else
            {
                VRNoticeSurface = NewObject<UWidgetComponent>(this, TEXT("VRReturnNotices"));
                VRNoticeSurface->SetWidgetSpace(EWidgetSpace::World);
                VRNoticeSurface->SetDrawSize(FVector2D(1280,720));
                VRNoticeSurface->SetBlendMode(EWidgetBlendMode::Transparent);
                VRNoticeSurface->SetTwoSided(true);
                VRNoticeSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                VRNoticeSurface->SetCastShadow(false);
                VRNoticeSurface->SetRenderInMainPass(false);
                VRNoticeSurface->SetTickWhenOffscreen(true);
                VRNoticeSurface->SetRedrawTime(1.0f/30.0f);
                VRNoticeSurface->SetWidget(WhaleObservationUI);
                VRNoticeSurface->RegisterComponent();
                VRNoticeLayer = NewObject<UExplorationNoticeLayer>(this, TEXT("VRNoticeLayer"));
                VRNoticeLayer->bLiveTexture = true;
                VRNoticeLayer->bSupportsDepth = false;
                VRNoticeLayer->bNoAlphaChannel = false;
                VRNoticeLayer->SetQuadSize(FVector2D(153.6f,86.4f));
                VRNoticeLayer->SetRelativeLocation(FVector(200,0,0));
                VRNoticeLayer->SetRelativeRotation(FRotator(0,180,0));
                VRNoticeLayer->SetPriority(100);
                VRNoticeLayer->RegisterComponent();
            }
		}
		else
		{
			UE_LOG(LogMqttVehicle, Error, TEXT("Could not spawn whale observation UI."));
		}
	}
}

void AMqttVehiclePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WhaleObservationUI)
	{
		WhaleObservation->OnObservationStateChanged.RemoveDynamic(WhaleObservationUI, &UWhaleObservationWidget::ApplyObservationState);
		ExplorationReturn->OnReturnStateChanged.RemoveDynamic(WhaleObservationUI, &UWhaleObservationWidget::ApplyReturnState);
		WhaleObservationUI->RemoveFromParent();
		WhaleObservationUI = nullptr;
	}
	if (VRNoticeLayer) { VRNoticeLayer->DestroyComponent(); VRNoticeLayer = nullptr; }
	if (VRNoticeSurface) { VRNoticeSurface->SetWidget(nullptr); VRNoticeSurface->DestroyComponent(); VRNoticeSurface = nullptr; }
	Super::EndPlay(EndPlayReason);
}

void AMqttVehiclePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (IsLocalPlayerController() && InputComponent)
	{
		InputComponent->BindKey(EKeys::H, IE_Pressed, this, &AMqttVehiclePlayerController::RequestExplorationReturn);
		InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AMqttVehiclePlayerController::ResumeManualExploration);
		auto& StopBinding = InputComponent->BindKey(EKeys::X, IE_Pressed, this, &AMqttVehiclePlayerController::StopExploration);
		StopBinding.bExecuteWhenPaused = true;
	}
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AMqttVehiclePlayerController::Tick(float Delta)
{
	Super::Tick(Delta);
    if (VRNoticeLayer && VRNoticeSurface && VRNoticeSurface->GetRenderTarget())
        VRNoticeLayer->SetTexture(VRNoticeSurface->GetRenderTarget());

	if (IsValid(VehicleUI))
	{
		const bool bShowVehicleHUD = IsValid(VehiclePawn) && !VehiclePawn->HasSubmersibleChoices();
		VehicleUI->SetVisibility(bShowVehicleHUD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShowVehicleHUD)
		{
			VehicleUI->UpdateSpeed(VehiclePawn->GetDriveForwardSpeed());
			VehicleUI->UpdateGear(VehiclePawn->GetDriveGear());
		}
	}
}

void AMqttVehiclePlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& HiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, HiddenComponents);
	if (IsValid(VehiclePawn) && VehiclePawn->SubmersibleView)
	{
		TArray<TWeakObjectPtr<UPrimitiveComponent>> Hidden;
		VehiclePawn->SubmersibleView->GetHiddenForView(VehiclePawn->IsInteriorCameraActive(), Hidden);
		for (auto Part : Hidden) if (Part.IsValid()) HiddenComponents.Add(Part->GetPrimitiveSceneId());
	}
}

void AMqttVehiclePlayerController::RequestExplorationReturn()
{
	ExplorationReturn->RequestReturn();
}

void AMqttVehiclePlayerController::StopExploration()
{
	ExplorationReturn->StopExploration();
}

void AMqttVehiclePlayerController::ResumeManualExploration()
{
	ExplorationReturn->ResumeManual();
}

void AMqttVehiclePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<AMqttVehiclePawn>(InPawn);
	VehiclePawnClass = VehiclePawn->GetClass();

	// subscribe to the pawn's OnDestroyed delegate
	VehiclePawn->OnDestroyed.AddDynamic(this, &AMqttVehiclePlayerController::OnPawnDestroyed);
}

void AMqttVehiclePlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// spawn a vehicle at the player start
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();

		if (AMqttVehiclePawn* RespawnedVehicle = GetWorld()->SpawnActor<AMqttVehiclePawn>(VehiclePawnClass, SpawnTransform))
		{
			// possess the vehicle
			Possess(RespawnedVehicle);
		}
	}
}

bool AMqttVehiclePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
