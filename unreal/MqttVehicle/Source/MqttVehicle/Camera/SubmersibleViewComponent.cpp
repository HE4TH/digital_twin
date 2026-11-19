#include "SubmersibleViewComponent.h"
#include "SubmersibleViewWidget.h"
#include "QuestDashboardComponent.h"
#include "MqttVehiclePawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"
#include "Camera/PlayerCameraManager.h"
#if WITH_DMIX_DESKTOP_VIDEO
#include "IPixelStreaming2Module.h"
#include "IPixelStreaming2Streamer.h"
#include "IPixelStreaming2VideoProducer.h"
#include "PixelCaptureInputFrameRHI.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "TextureResource.h"

namespace
{
class FDashboardVideoProducer final : public IPixelStreaming2VideoProducer
{
public:
	virtual EVideoProducerCapabilities GetCapabilities() override { return EVideoProducerCapabilities::Default; }
	virtual FString ToString() override { return TEXT("Dashboard spectator 1280x720"); }
};
}
#endif

USubmersibleViewComponent::USubmersibleViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void USubmersibleViewComponent::BeginPlay()
{
	Super::BeginPlay();
	Vehicle = Cast<AMqttVehiclePawn>(GetOwner());
	if (!Vehicle.IsValid() || !Vehicle->HasSubmersibleChoices()) { SetComponentTickEnabled(false); return; }
	BuildCockpit();
	BuildLanzamanLighting();
	AddTickPrerequisiteComponent(Vehicle->GetFrontSpringArm());
	AddTickPrerequisiteComponent(Vehicle->GetBackSpringArm());
#if PLATFORM_ANDROID
	auto* QuestDashboard = NewObject<UQuestDashboardComponent>(GetOwner());
	GetOwner()->AddInstanceComponent(QuestDashboard);
	QuestDashboard->RegisterComponent();
#endif
}

void USubmersibleViewComponent::BuildCockpit()
{
	// Iris has no furnished cabin. Lanzaman instead uses its original two-seat interior.
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* Dark = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Underwater/Materials/M_RoverWindow.M_RoverWindow"));
	UMaterialInterface* Rim = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Underwater/Materials/M_RoverHullYellow.M_RoverHullYellow"));
	auto Box = [&](const TCHAR* Name, FVector Position, FVector Size, FRotator Rotation, UMaterialInterface* Material)
	{
		auto* Part = NewObject<UStaticMeshComponent>(GetOwner(), MakeUniqueObjectName(GetOwner(), UStaticMeshComponent::StaticClass(), Name));
		Part->SetupAttachment(Vehicle->GetMesh());
		Part->SetStaticMesh(Cube);
		Part->SetMaterial(0, Material);
		Part->SetRelativeLocation(Position);
		Part->SetRelativeRotation(Rotation);
		Part->SetRelativeScale3D(Size / 100.0);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCanEverAffectNavigation(false);
		Part->SetCastShadow(false);
		Part->RegisterComponent();
		CockpitParts.Add(Part);
		return Part;
	};
	// Eye at (35,0,155); a wide oval pressure-window frame is fixed to the hull.
	for (int32 I = 0; I < 32; ++I)
	{
		const double A = 2.0 * PI * I / 32.0;
		const FVector P(150, 132 * FMath::Cos(A), 155 + 100 * FMath::Sin(A));
		const FVector Tangent(0, -132 * FMath::Sin(A), 100 * FMath::Cos(A));
		Box(TEXT("Cockpit_WindowRim"), P, FVector(29,10,10), Tangent.Rotation(), Rim);
	}
	Box(TEXT("Cockpit_Dashboard"), FVector(115,0,104), FVector(80,224,16), FRotator(-12,0,0), Dark);
	Box(TEXT("Cockpit_LeftWall"), FVector(5,-151,122), FVector(280,24,230), FRotator::ZeroRotator, Dark);
	Box(TEXT("Cockpit_RightWall"), FVector(5,151,122), FVector(280,24,230), FRotator::ZeroRotator, Dark);
	Box(TEXT("Cockpit_Roof"), FVector(0,0,263), FVector(285,320,18), FRotator::ZeroRotator, Dark);
	Box(TEXT("Cockpit_Floor"), FVector(0,0,22), FVector(285,320,18), FRotator::ZeroRotator, Dark);
	Box(TEXT("Cockpit_Rear"), FVector(-132,0,130), FVector(18,320,250), FRotator::ZeroRotator, Dark);
	for (float Side : {-1.0f, 1.0f})
	{
		Box(TEXT("Cockpit_ControlBase"), FVector(100,Side*45,122), FVector(24,24,8), FRotator::ZeroRotator, Rim);
		Box(TEXT("Cockpit_ControlStick"), FVector(100,Side*45,136), FVector(6,6,24), FRotator(-12,0,0), Dark);
	}
	auto* Label = NewObject<UTextRenderComponent>(GetOwner(), TEXT("Cockpit_ConsoleLabel"));
	Label->SetupAttachment(Vehicle->GetMesh());
	Label->SetRelativeLocation(FVector(131,0,126));
	Label->SetRelativeRotation(FRotator(0,180,0));
	Label->SetText(FText::FromString(TEXT("DEEP SEA\nEXPLORER")));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetWorldSize(3.5f);
	Label->SetTextRenderColor(FColor(100,240,230));
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Label->SetCastShadow(false);
	Label->RegisterComponent();
	CockpitParts.Add(Label);
}

void USubmersibleViewComponent::BuildLanzamanLighting()
{
	// The mesh contains both seats and the remaining lower display with its screen graphics.
	CabinLight = NewObject<UPointLightComponent>(GetOwner(), TEXT("Lanzaman_CabinLight"));
	CabinLight->SetupAttachment(Vehicle->SubmersibleHull);
	CabinLight->SetRelativeLocation(FVector(-35,0,300));
	CabinLight->SetIntensityUnits(ELightUnits::Lumens);
	CabinLight->SetIntensity(220.0f);
	CabinLight->SetAttenuationRadius(290.0f);
	CabinLight->SetLightColor(FLinearColor(0.65f,0.85f,1.0f));
	CabinLight->SetCastShadows(false);
	CabinLight->RegisterComponent();
	RefreshInterior();
}

void USubmersibleViewComponent::RefreshInterior()
{
	const bool bLanzaman = Vehicle.IsValid() && Vehicle->GetSubmersibleVariant() == 1;
	if (CabinLight) CabinLight->SetVisibility(bLanzaman);
}

void USubmersibleViewComponent::GetHiddenForView(bool bInterior, TArray<TWeakObjectPtr<UPrimitiveComponent>>& Out) const
{
	if (!Vehicle.IsValid() || CockpitParts.IsEmpty()) return;
	TInlineComponentArray<UPrimitiveComponent*> Parts(Vehicle.Get());
	for (UPrimitiveComponent* Part : Parts)
	{
		const bool bCockpit = CockpitParts.Contains(Part);
		const bool bLanzaman = Vehicle->GetSubmersibleVariant() == 1;
		const bool bCabinPart = bLanzaman ? Part == Vehicle->SubmersibleHull : bCockpit;
		if ((bInterior && !bCabinPart) || (!bInterior && bCockpit)) Out.AddUnique(Part);
	}
}

void USubmersibleViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	AMqttVehiclePawn* Pawn = Vehicle.Get();
	auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->IsLocalPlayerController() || !FApp::CanEverRender()) { ReleaseFeed(); return; }
	UpdateSpectator(DeltaTime, PC);
#if PLATFORM_ANDROID
	// A second scene capture and flat-screen overlay are unnecessary in standalone VR.
	return;
#endif
	if (!Capture)
	{
		if (!FeedMaterial) return;
		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		RenderTarget->ClearColor = FLinearColor::Black;
		RenderTarget->InitCustomFormat(640,360,PF_B8G8R8A8,false);
		Capture = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("OppositeCameraCapture"));
		Capture->bCaptureEveryFrame = false;
		Capture->bCaptureOnMovement = false;
		Capture->bAlwaysPersistRenderingState = true;
		Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture->TextureTarget = RenderTarget;
		Capture->ShowFlags.SetMotionBlur(false);
		Capture->RegisterComponent();
		FeedInstance = UMaterialInstanceDynamic::Create(FeedMaterial, this);
		FeedInstance->SetTextureParameterValue(TEXT("CameraFeed"), RenderTarget);
		Widget = CreateWidget<USubmersibleViewWidget>(PC);
		if (Widget) { Widget->AddToPlayerScreen(5); Widget->SetFeed(FeedInstance); }
		CaptureElapsed = 1.0f;
	}
	const bool bInteriorMain = Pawn->IsInteriorCameraActive();
	CaptureElapsed += DeltaTime;
	if (CaptureElapsed < 1.0f / 20.0f && bLastInterior == bInteriorMain) return;
	CaptureElapsed = 0.0f;
	bLastInterior = bInteriorMain;
	UCameraComponent* Other = bInteriorMain ? Pawn->GetBackCamera() : Pawn->GetFollowCamera();
	Capture->SetWorldLocationAndRotation(Other->GetComponentLocation(), Other->GetComponentRotation());
	Capture->FOVAngle = Other->FieldOfView;
	Capture->PostProcessSettings = Other->PostProcessSettings;
	Capture->PostProcessBlendWeight = Other->PostProcessBlendWeight;
	Capture->HiddenComponents.Reset();
	GetHiddenForView(!bInteriorMain, Capture->HiddenComponents);
	Capture->CaptureScene();
}

void USubmersibleViewComponent::ReleaseFeed()
{
	ReleaseSpectator();
	if (Widget) { Widget->RemoveFromParent(); Widget = nullptr; }
	if (Capture) { Capture->TextureTarget = nullptr; Capture->DestroyComponent(); Capture = nullptr; }
	FeedInstance = nullptr;
	RenderTarget = nullptr;
}

void USubmersibleViewComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	ReleaseFeed();
	for (auto Part : CockpitParts) if (Part) Part->DestroyComponent();
	CockpitParts.Reset();
	if (CabinLight) { CabinLight->DestroyComponent(); CabinLight = nullptr; }
	Super::EndPlay(Reason);
}

void USubmersibleViewComponent::UpdateSpectator(float DeltaTime, APlayerController* PC)
{
#if WITH_DMIX_DESKTOP_VIDEO
	// GamePreview automation worlds must never create network/rendering side effects.
	if (!GetWorld() || (GetWorld()->WorldType != EWorldType::PIE && GetWorld()->WorldType != EWorldType::Game)) return;
	IPixelStreaming2Module& Module = IPixelStreaming2Module::Get();
	if (!Module.IsReady()) return;
	auto Streamer = Module.FindStreamer(Module.GetDefaultStreamerID());
	if (!Streamer || !Streamer->IsStreaming()) { ReleaseSpectator(); return; }
	if (SpectatorStreamer != Streamer)
	{
		ReleaseSpectator();
		SpectatorStreamer = Streamer;
		PreviousProducer = Streamer->GetVideoProducer().Pin();
		PreviousStreamFPS = Streamer->GetStreamFPS();
		SpectatorProducer = MakeShared<FDashboardVideoProducer>();
		SpectatorTarget = NewObject<UTextureRenderTarget2D>(this);
		SpectatorTarget->ClearColor = FLinearColor::Black;
		SpectatorTarget->InitCustomFormat(1280,720,PF_B8G8R8A8,false);
		SpectatorCapture = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("DashboardSpectatorCapture"));
		SpectatorCapture->bCaptureEveryFrame = false;
		SpectatorCapture->bCaptureOnMovement = false;
		SpectatorCapture->bAlwaysPersistRenderingState = true;
		SpectatorCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		SpectatorCapture->TextureTarget = SpectatorTarget;
		SpectatorCapture->FOVAngle = 100.0f;
		SpectatorCapture->ShowFlags.SetMotionBlur(false);
		SpectatorCapture->ShowFlags.SetTemporalAA(false);
		SpectatorCapture->RegisterComponent();
		Streamer->SetVideoProducer(SpectatorProducer);
		Streamer->SetStreamFPS(30);
		SpectatorElapsed = 1.0f;
		UE_LOG(LogTemp, Log, TEXT("DASHBOARD_SPECTATOR_READY: 1280x720, 30fps maximum, horizontal FOV 100, capture only with viewers"));
	}
	// No extra scene rendering while nobody is watching the web dashboard.
	if (Streamer->GetConnectedPlayers().IsEmpty() || !PC->PlayerCameraManager || GetWorld()->IsPaused()) return;
	SpectatorElapsed += DeltaTime;
	if (SpectatorElapsed < 1.0f/30.0f) return;
	SpectatorElapsed = 0.0f;
	SpectatorCapture->SetWorldLocationAndRotation(PC->PlayerCameraManager->GetCameraLocation(), PC->PlayerCameraManager->GetCameraRotation());
	const bool bInterior = Vehicle->IsInteriorCameraActive();
	UCameraComponent* MainCamera = bInterior ? Vehicle->GetFollowCamera() : Vehicle->GetBackCamera();
	SpectatorCapture->PostProcessSettings = MainCamera->PostProcessSettings;
	SpectatorCapture->PostProcessBlendWeight = MainCamera->PostProcessBlendWeight;
	SpectatorCapture->HiddenComponents.Reset();
	GetHiddenForView(bInterior, SpectatorCapture->HiddenComponents);
	SpectatorCapture->CaptureScene();
	FTextureRenderTargetResource* TargetResource = SpectatorTarget->GameThread_GetRenderTargetResource();
	const auto Producer = SpectatorProducer;
	// CaptureScene enqueues rendering first. Push the resulting GPU frame afterwards.
	ENQUEUE_RENDER_COMMAND(PushDashboardSpectatorFrame)([TargetResource, Producer](FRHICommandListImmediate& RHICmdList)
	{
		if (FTextureRHIRef Texture = TargetResource->GetRenderTargetTexture())
		{
			Producer->PushFrame(FPixelCaptureInputFrameRHI(Texture));
		}
	});
#endif
}

void USubmersibleViewComponent::ReleaseSpectator()
{
#if WITH_DMIX_DESKTOP_VIDEO
	if (SpectatorStreamer && SpectatorStreamer->GetVideoProducer().Pin() == SpectatorProducer)
	{
		SpectatorStreamer->SetVideoProducer(PreviousProducer);
		SpectatorStreamer->SetStreamFPS(PreviousStreamFPS);
	}
	if (SpectatorCapture)
	{
		SpectatorCapture->TextureTarget = nullptr;
		SpectatorCapture->DestroyComponent();
		SpectatorCapture = nullptr;
		// Drain frame submissions before allowing the target resource to be collected.
		FlushRenderingCommands();
	}
	SpectatorTarget = nullptr;
	SpectatorProducer.Reset();
	PreviousProducer.Reset();
	SpectatorStreamer.Reset();
#endif
}
