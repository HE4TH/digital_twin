#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestVideoEncoder.h"
#include "QuestDashboardComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class IHttpRequest;
struct FQuestFrameReadback;

/** Viewer-gated spectator video with bounded GPU readback for the standalone headset. */
UCLASS()
class MQTTVEHICLE_API UQuestDashboardComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UQuestDashboardComponent();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* Function) override;
private:
	void PollDemand();
	void CaptureFrame();
	void UploadFrame(TArray<uint8> Bytes);
 void UploadVideo();
 TSharedPtr<FQuestVideoEncoder, ESPMode::ThreadSafe> VideoEncoder;
 TArray<FQuestVideoPacket> VideoPackets;
 FString VideoSession;
 bool bVideo = false, bEncoding = false, bNeedKeyFrame = true;
 int32 QueuedVideoBytes = 0;
	UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> Capture;
	UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> Target;
	TSharedPtr<FQuestFrameReadback, ESPMode::ThreadSafe> Frame;
 TArray<TSharedPtr<FQuestFrameReadback, ESPMode::ThreadSafe>> WaitingFrames;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> DemandRequest;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> UploadRequest;
	FString BaseURL;
    int32 FrameWidth = 960;
    int32 FrameHeight = 540;
    int32 JPEGQuality = 80;
    float FrameRate = 10.0f;
	FString LastActionId;
	FString LastActionMessage;
	double LastControlAt = 0;
	double NextPoll = 0;
	double NextCapture = 0;
	bool bWanted = false;
	bool bBusy = false;
	bool bEnding = false;
};

