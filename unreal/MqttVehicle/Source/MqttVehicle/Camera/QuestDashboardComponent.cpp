#include "QuestDashboardComponent.h"
#include "MqttVehiclePawn.h"
#include "MqttVehiclePlayerController.h"
#include "WhaleObservationWidget.h"
#include "MqttVehicleBridgeSubsystem.h"
#include "ExplorationReturnComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SubmersibleViewComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Async/Async.h"
#include "Misc/ConfigCacheIni.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIGPUReadback.h"
#include "TextureResource.h"


struct FQuestFrameReadback
{
	FRHIGPUTextureReadback Readback{TEXT("QuestDashboard")};
	std::atomic<bool> bQueued{false};
};

UQuestDashboardComponent::UQuestDashboardComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UQuestDashboardComponent::BeginPlay()
{
	Super::BeginPlay();
#if PLATFORM_ANDROID
	bool bEnabled = true;
	GConfig->GetBool(TEXT("DmixQuestDashboard"), TEXT("Enabled"), bEnabled, GGameIni);
	GConfig->GetString(TEXT("DmixQuestDashboard"), TEXT("URL"), BaseURL, GGameIni);
	if (!bEnabled || BaseURL.IsEmpty()) { SetComponentTickEnabled(false); return; }
	if (auto* Bridge = GetWorld()->GetSubsystem<UMqttVehicleBridgeSubsystem>()) Bridge->bDashboardOutputEnabled = false;
	BaseURL.RemoveFromEnd(TEXT("/"));
    GConfig->GetInt(TEXT("DmixQuestDashboard"), TEXT("Width"), FrameWidth, GGameIni);
    GConfig->GetInt(TEXT("DmixQuestDashboard"), TEXT("JPEGQuality"), JPEGQuality, GGameIni);
    GConfig->GetFloat(TEXT("DmixQuestDashboard"), TEXT("FrameRate"), FrameRate, GGameIni);
    FrameWidth = (FMath::Clamp(FrameWidth, 640, 1280) / 32) * 32;
    FrameHeight = FrameWidth * 9 / 16;
    JPEGQuality = FMath::Clamp(JPEGQuality, 50, 90);
    FrameRate = FMath::IsFinite(FrameRate) ? FMath::Clamp(FrameRate, 1.0f, 15.0f) : 10.0f;
	GConfig->GetBool(TEXT("DmixQuestDashboard"), TEXT("H264"), bVideo, GGameIni);
 if (bVideo)
 {
  FrameRate=30; VideoSession=FGuid::NewGuid().ToString(EGuidFormats::Digits);
  VideoEncoder=MakeShared<FQuestVideoEncoder,ESPMode::ThreadSafe>(FrameWidth,FrameHeight,30);
 }
 FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	UE_LOG(LogTemp, Log, TEXT("QUEST_DASHBOARD_READY: %s, %dx%d at most %.1ffps (JPEG fallback quality=%d) when dashboard is open"), *BaseURL, FrameWidth, FrameHeight, FrameRate, JPEGQuality);
#else
	SetComponentTickEnabled(false);
#endif
}

void UQuestDashboardComponent::PollDemand()
{
    auto* Bridge = GetWorld()->GetSubsystem<UMqttVehicleBridgeSubsystem>();
    auto* Pawn = Cast<AMqttVehiclePawn>(GetOwner());
    auto* Controller = Pawn ? Pawn->GetController() : nullptr;
    auto* Return = Controller ? Controller->FindComponentByClass<UExplorationReturnComponent>() : nullptr;
    const auto Data = MakeShared<FJsonObject>();
    Data->SetBoolField(TEXT("send_enabled"), Bridge && Bridge->bDashboardOutputEnabled);
    Data->SetBoolField(TEXT("broker"), Bridge && Bridge->IsBrokerConnected());
    Data->SetBoolField(TEXT("can_return"), Return && Return->GetReturnState().bCanReturn);
    Data->SetStringField(TEXT("mode"), Return ? Return->GetReturnState().Detail.ToString() : TEXT("대기"));
    TArray<TSharedPtr<FJsonValue>> Captions;
    FString Badge;
    if (const auto* PC = Cast<AMqttVehiclePlayerController>(Controller))
        if (const auto* Widget = PC->GetObservationWidget())
        {
            TArray<FExplorationVisibleNotice> Notices; Widget->GetVisibleNotices(Notices, Badge);
            for (const auto& Notice : Notices)
            {
                auto Item = MakeShared<FJsonObject>();
                Item->SetStringField(TEXT("kind"), Notice.Kind);
                Item->SetStringField(TEXT("text"), Notice.Text);
                Item->SetNumberField(TEXT("remaining"), Notice.RemainingSeconds);
                Captions.Add(MakeShared<FJsonValueObject>(Item));
            }
        }
    Data->SetArrayField(TEXT("notices"), Captions);
    Data->SetStringField(TEXT("notice_badge"), Badge);
    if (!LastActionId.IsEmpty())
    {
        const auto Ack = MakeShared<FJsonObject>();
        Ack->SetStringField(TEXT("id"), LastActionId);
        Ack->SetStringField(TEXT("message"), LastActionMessage);
        Data->SetObjectField(TEXT("ack"), Ack);
    }
    FString Body;
    FJsonSerializer::Serialize(Data, TJsonWriterFactory<>::Create(&Body));
    DemandRequest = FHttpModule::Get().CreateRequest();
    DemandRequest->SetURL(BaseURL + TEXT("/api/quest/control"));
    DemandRequest->SetVerb(TEXT("POST"));
    DemandRequest->SetHeader(TEXT("X-Quest-Stream"), TEXT("1"));
    DemandRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    DemandRequest->SetContentAsString(Body);
    DemandRequest->SetTimeout(2.0f);
    DemandRequest->OnProcessRequestComplete().BindWeakLambda(this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bOK)
    {
        DemandRequest.Reset();
        TSharedPtr<FJsonObject> Reply;
        if (!bOK || !Response.IsValid() || Response->GetResponseCode() != 200
            || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Reply) || !Reply)
        { bWanted = false; return; }
        LastControlAt = FPlatformTime::Seconds();
        Reply->TryGetBoolField(TEXT("wanted"), bWanted);
        const TSharedPtr<FJsonObject>* Action = nullptr;
        if (!Reply->TryGetObjectField(TEXT("action"), Action) || !Action || !Action->IsValid()) return;
        FString Id, Name;
        if (!(*Action)->TryGetStringField(TEXT("id"), Id) || Id.IsEmpty() || Id == LastActionId
            || !(*Action)->TryGetStringField(TEXT("action"), Name)) return;
        LastActionId = Id; // A retry must never trigger the return twice.
        LastActionMessage = TEXT("요청 적용 불가");
        auto* Output = GetWorld()->GetSubsystem<UMqttVehicleBridgeSubsystem>();
        if (Output && (Name == TEXT("send_on") || Name == TEXT("send_off")))
        {
            Output->bDashboardOutputEnabled = Name == TEXT("send_on");
            LastActionMessage = Output->bDashboardOutputEnabled ? TEXT("RC 송신 켜짐") : TEXT("RC 송신 꺼짐 · 차량 정지");
        }
        else if (Name == TEXT("return"))
        {
            auto* Vehicle = Cast<AMqttVehiclePawn>(GetOwner());
            auto* PC = Vehicle ? Vehicle->GetController() : nullptr;
            auto* Exploration = PC ? PC->FindComponentByClass<UExplorationReturnComponent>() : nullptr;
            if (Exploration && Exploration->GetReturnState().bCanReturn)
            {
                Exploration->RequestReturn();
                LastActionMessage = Exploration->GetReturnState().Detail.ToString();
            }
            else LastActionMessage = TEXT("현재 자동복귀를 시작할 수 없습니다");
        }
        UE_LOG(LogTemp, Log, TEXT("QUEST_DASHBOARD_ACTION: %s %s"), *Name, *LastActionMessage);
    });
    if (!DemandRequest->ProcessRequest()) { DemandRequest.Reset(); bWanted = false; }
}

void UQuestDashboardComponent::CaptureFrame()
{
	auto* Vehicle = Cast<AMqttVehiclePawn>(GetOwner());
	auto* PC = Vehicle ? Cast<APlayerController>(Vehicle->GetController()) : nullptr;
	if (!PC || !PC->PlayerCameraManager || GetWorld()->IsPaused()) return;
	if (!Capture)
	{
		Target = NewObject<UTextureRenderTarget2D>(this);
		Target->ClearColor = FLinearColor::Black;
		Target->InitCustomFormat(FrameWidth, FrameHeight, PF_B8G8R8A8, false);
		Capture = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("QuestDashboardCapture"));
		Capture->bCaptureEveryFrame = false;
		Capture->bCaptureOnMovement = false;
		Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture->TextureTarget = Target;
		Capture->FOVAngle = 100.0f;
		Capture->ShowFlags.SetMotionBlur(false);
		Capture->ShowFlags.SetTemporalAA(false);
		Capture->RegisterComponent();
	}
	Capture->SetWorldLocationAndRotation(PC->PlayerCameraManager->GetCameraLocation(), PC->PlayerCameraManager->GetCameraRotation());
	const bool bInterior = Vehicle->IsInteriorCameraActive();
	UCameraComponent* Camera = bInterior ? Vehicle->GetFollowCamera() : Vehicle->GetBackCamera();
	Capture->PostProcessSettings = Camera->PostProcessSettings;
	Capture->PostProcessBlendWeight = Camera->PostProcessBlendWeight;
	Capture->HiddenComponents.Reset();
	if (auto* View = Vehicle->FindComponentByClass<USubmersibleViewComponent>())
		View->GetHiddenForView(bInterior, Capture->HiddenComponents);
	const auto Pending = MakeShared<FQuestFrameReadback, ESPMode::ThreadSafe>();
 if (Frame) WaitingFrames.Add(Pending); else Frame = Pending;
	bBusy = true;
	Capture->CaptureScene();
	FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource();
	ENQUEUE_RENDER_COMMAND(ReadQuestDashboardFrame)([Pending, Resource](FRHICommandListImmediate& Cmd)
	{
		Pending->Readback.EnqueueCopy(Cmd, Resource->GetRenderTargetTexture());
		Pending->bQueued.store(true);
	});
}

void UQuestDashboardComponent::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* Function)
{
	Super::TickComponent(DeltaTime, Tick, Function);
	if (bEnding || BaseURL.IsEmpty()) return;
	const double Now = FPlatformTime::Seconds();
    if (Now - LastControlAt > 3.0)
        if (auto* Bridge = GetWorld()->GetSubsystem<UMqttVehicleBridgeSubsystem>()) Bridge->bDashboardOutputEnabled = false;
	if (Now >= NextPoll && !DemandRequest) { NextPoll = Now + 0.5; PollDemand(); }
	if (Frame && !bEncoding && Frame->bQueued.load() && Frame->Readback.IsReady())
	{
		int32 Pitch = 0;
		const auto* Source = static_cast<const FColor*>(Frame->Readback.Lock(Pitch));
		TArray<FColor> Pixels;
		if (Source && Pitch >= FrameWidth)
		{
			Pixels.SetNumUninitialized(FrameWidth * FrameHeight);
			for (int32 Row = 0; Row < FrameHeight; ++Row)
				FMemory::Memcpy(Pixels.GetData() + Row*FrameWidth, Source + Row*Pitch, FrameWidth*sizeof(FColor));
		}
		Frame->Readback.Unlock();
		Frame.Reset();
  if (!WaitingFrames.IsEmpty()) { Frame = WaitingFrames[0]; WaitingFrames.RemoveAt(0); }
		if (Pixels.IsEmpty()) { bBusy = false; return; }
		TWeakObjectPtr<UQuestDashboardComponent> Weak(this);
  if(bVideo)
  {
   bEncoding=true; const auto Encoder=VideoEncoder; const bool ForceKey=bNeedKeyFrame;
   const int64 Timestamp=static_cast<int64>(Now*1000000.0);
   Async(EAsyncExecution::ThreadPool,[Weak, Encoder, Pixels=MoveTemp(Pixels), Timestamp, ForceKey]() mutable
   {
    TArray<FQuestVideoPacket> Packets; const bool OK=Encoder->Encode(Pixels,Timestamp,ForceKey,Packets);
    AsyncTask(ENamedThreads::GameThread,[Weak,OK,Packets=MoveTemp(Packets)]() mutable
    {
     auto* Self=Weak.Get(); if(!Self || Self->bEnding) return;
     Self->bEncoding=false; Self->bBusy=false;
     if(!OK)
     {
      UE_LOG(LogTemp,Warning,TEXT("QUEST_VIDEO_FALLBACK: hardware AVC unavailable; JPEG at 10fps"));
      if (Self->UploadRequest) { Self->UploadRequest->OnProcessRequestComplete().Unbind(); Self->UploadRequest->CancelRequest(); Self->UploadRequest.Reset(); }
       Self->bVideo=false; Self->VideoEncoder.Reset(); Self->VideoPackets.Reset(); Self->QueuedVideoBytes=0; Self->FrameRate=10;
      return;
     }
     for(auto& Packet:Packets)
     {
      if(!Self->bWanted) { Self->bNeedKeyFrame=true; continue; }
      if(Self->QueuedVideoBytes+Packet.Data.Num()>1048576 || Self->VideoPackets.Num()>=20)
      { Self->VideoPackets.Reset(); Self->QueuedVideoBytes=0; Self->bNeedKeyFrame=true; Self->VideoSession=FGuid::NewGuid().ToString(EGuidFormats::Digits); }
      if(Self->bNeedKeyFrame && !Packet.bKeyFrame) continue;
      Self->bNeedKeyFrame=false; Self->QueuedVideoBytes+=Packet.Data.Num(); Self->VideoPackets.Add(MoveTemp(Packet));
     }
     Self->UploadVideo();
    });
   });
  }
  else
		Async(EAsyncExecution::ThreadPool, [Weak, Pixels = MoveTemp(Pixels), Width = FrameWidth, Height = FrameHeight, Quality = JPEGQuality]() mutable
		{
			auto& Module = FModuleManager::GetModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
			auto JPEG = Module.CreateImageWrapper(EImageFormat::JPEG);
			TArray<uint8> Bytes;
			if (JPEG && JPEG->SetRaw(Pixels.GetData(), Pixels.Num()*sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8))
			{
				const auto& Compressed = JPEG->GetCompressed(Quality);
				Bytes.Append(Compressed.GetData(), Compressed.Num());
			}
			AsyncTask(ENamedThreads::GameThread, [Weak, Bytes = MoveTemp(Bytes)]() mutable
			{
				if (auto* Self = Weak.Get(); Self && !Self->bEnding) Self->UploadFrame(MoveTemp(Bytes));
			});
		});
	}
	if (bWanted && (bVideo ? WaitingFrames.Num() + (Frame ? 1 : 0) < 3 : !bBusy && !Frame) && Now >= NextCapture)
	{
		NextCapture = FMath::Max(NextCapture + 1.0 / FrameRate, Now);
		CaptureFrame();
	}
}

void UQuestDashboardComponent::UploadFrame(TArray<uint8> Bytes)
{
	if (Bytes.IsEmpty() || !bWanted) { bBusy = false; return; }
	UploadRequest = FHttpModule::Get().CreateRequest();
	UploadRequest->SetURL(BaseURL + TEXT("/api/quest/frame"));
	UploadRequest->SetVerb(TEXT("POST"));
	UploadRequest->SetHeader(TEXT("X-Quest-Stream"), TEXT("1"));
	UploadRequest->SetHeader(TEXT("Content-Type"), TEXT("image/jpeg"));
	UploadRequest->SetContent(MoveTemp(Bytes));
	UploadRequest->SetTimeout(2.0f);
	UploadRequest->OnProcessRequestComplete().BindWeakLambda(this,
		[this](FHttpRequestPtr, FHttpResponsePtr Response, bool bOK)
	{
		if (!bOK || !Response.IsValid() || Response->GetResponseCode() != 200) bWanted = false;
		bBusy = false;
		UploadRequest.Reset();
	});
	if (!UploadRequest->ProcessRequest()) { UploadRequest.Reset(); bBusy = false; bWanted = false; }
}

void UQuestDashboardComponent::UploadVideo()
{
 if(UploadRequest || VideoPackets.IsEmpty() || !bWanted || bEnding) return;
 TArray<uint8> Body;
 for(const auto& Packet:VideoPackets)
 {
  const uint32 Size=Packet.Data.Num();
  Body.Append(reinterpret_cast<const uint8*>(&Packet.TimestampUs),8);
  Body.Append(reinterpret_cast<const uint8*>(&Size),4); Body.Append(Packet.Data);
 }
 VideoPackets.Reset(); QueuedVideoBytes=0;
 UploadRequest=FHttpModule::Get().CreateRequest();
 UploadRequest->SetURL(BaseURL+TEXT("/api/quest/video")); UploadRequest->SetVerb(TEXT("POST"));
 UploadRequest->SetHeader(TEXT("X-Quest-Stream"),TEXT("1"));
 UploadRequest->SetHeader(TEXT("Content-Type"),TEXT("application/octet-stream"));
 UploadRequest->SetHeader(TEXT("X-Video-Session"),VideoSession);
 UploadRequest->SetHeader(TEXT("X-Video-Width"),FString::FromInt(FrameWidth));
 UploadRequest->SetHeader(TEXT("X-Video-Height"),FString::FromInt(FrameHeight));
 UploadRequest->SetContent(MoveTemp(Body)); UploadRequest->SetTimeout(1.0f);
 UploadRequest->OnProcessRequestComplete().BindWeakLambda(this,[this](FHttpRequestPtr,FHttpResponsePtr Response,bool OK)
 {
  UploadRequest.Reset();
  if(!OK || !Response || Response->GetResponseCode()!=200)
  { VideoPackets.Reset(); QueuedVideoBytes=0; bNeedKeyFrame=true; VideoSession=FGuid::NewGuid().ToString(EGuidFormats::Digits); }
  else UploadVideo();
 });
 if(!UploadRequest->ProcessRequest())
 { UploadRequest.Reset(); bNeedKeyFrame=true; VideoSession=FGuid::NewGuid().ToString(EGuidFormats::Digits); }
}

void UQuestDashboardComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	bEnding = true;
	for (auto Request : {DemandRequest, UploadRequest})
		if (Request) { Request->OnProcessRequestComplete().Unbind(); Request->CancelRequest(); }
	DemandRequest.Reset();
	UploadRequest.Reset();
	if (Capture) { Capture->TextureTarget = nullptr; Capture->DestroyComponent(); Capture = nullptr; }
	FlushRenderingCommands();
	Frame.Reset();
 VideoEncoder.Reset(); VideoPackets.Reset(); WaitingFrames.Reset();
	Target = nullptr;
	Super::EndPlay(Reason);
}


