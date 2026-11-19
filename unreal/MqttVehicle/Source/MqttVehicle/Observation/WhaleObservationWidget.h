#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/StereoLayerComponent.h"
#include "WhaleObservationComponent.h"
#include "ExplorationReturnComponent.h"
#include "WhaleObservationWidget.generated.h"


struct FExplorationVisibleNotice
{
 FString Kind;
 FString Text;
 float RemainingSeconds = 0;
};

UCLASS()
class MQTTVEHICLE_API UExplorationNoticeLayer : public UStereoLayerComponent
{
    GENERATED_BODY()
public:
    UExplorationNoticeLayer() { StereoLayerType = SLT_FaceLocked; }
};

/** Encounter notices and shortcuts; never consumes driving or camera input. */
UCLASS()
class MQTTVEHICLE_API UWhaleObservationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	explicit UWhaleObservationWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	void ApplyObservationState(const FWhaleObservationSnapshot& NewState);
	UFUNCTION()
	void ApplyReturnState(const FExplorationReturnSnapshot& NewState);
	bool bVRPresentation = false;
 void GetVisibleNotices(TArray<FExplorationVisibleNotice>& Notices, FString& Badge) const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
	friend class FEncounterNoticeTest;
	void RefreshVisibility();
	UPROPERTY(Transient) TObjectPtr<class UBorder> NoticePanel;
	UPROPERTY(Transient) TObjectPtr<class UTextBlock> WhaleNotice;
	UPROPERTY(Transient) TObjectPtr<class UTextBlock> JellyNotice;
	UPROPERTY(Transient) TObjectPtr<class UBorder> ReturnPanel;
	UPROPERTY(Transient) TObjectPtr<class UTextBlock> ReturnNotice;
	UPROPERTY(Transient) TObjectPtr<class UTextBlock> ReturnBadge;
	FText ReturnMessage;
	float ReturnSeconds = 0;
	float WhaleSeconds = 0, JellySeconds = 0;

	FWhaleObservationSnapshot State;
	FExplorationReturnSnapshot ReturnState;
};
