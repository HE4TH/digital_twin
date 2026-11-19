#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubmersibleViewComponent.generated.h"

class AMqttVehiclePawn;
class UPrimitiveComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USubmersibleViewWidget;
class UPointLightComponent;
class IPixelStreaming2Streamer;
class IPixelStreaming2VideoProducer;
class APlayerController;

/** A local cockpit and one opposite-camera feed. Neither camera changes vehicle control. */
UCLASS(ClassGroup=(Exploration))
class MQTTVEHICLE_API USubmersibleViewComponent : public UActorComponent
{

	GENERATED_BODY()
public:
	USubmersibleViewComponent();
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	TObjectPtr<UMaterialInterface> FeedMaterial;
	void GetHiddenForView(bool bInterior, TArray<TWeakObjectPtr<UPrimitiveComponent>>& Out) const;
	USceneCaptureComponent2D* GetCapture() const { return Capture; }
	int32 GetCockpitPartCount() const { return CockpitParts.Num(); }
	void RefreshInterior();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
private:
	void BuildCockpit();
	void BuildLanzamanLighting();
	void ReleaseFeed();
	void UpdateSpectator(float DeltaTime, APlayerController* PC);
	void ReleaseSpectator();
	UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> SpectatorCapture;
	UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> SpectatorTarget;
	TSharedPtr<IPixelStreaming2Streamer> SpectatorStreamer;
	TSharedPtr<IPixelStreaming2VideoProducer> SpectatorProducer;
	TSharedPtr<IPixelStreaming2VideoProducer> PreviousProducer;
	float SpectatorElapsed = 1.0f;
	int32 PreviousStreamFPS = 60;
	TWeakObjectPtr<AMqttVehiclePawn> Vehicle;
	UPROPERTY(Transient) TArray<TObjectPtr<UPrimitiveComponent>> CockpitParts;
	UPROPERTY(Transient) TObjectPtr<UPointLightComponent> CabinLight;
	UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> Capture;
	UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> RenderTarget;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> FeedInstance;
	UPROPERTY(Transient) TObjectPtr<USubmersibleViewWidget> Widget;
	float CaptureElapsed = 1.0f;
	bool bLastInterior = false;
};
