#include "WhaleObservationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "WhaleObservation"

UWhaleObservationWidget::UWhaleObservationWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> UWhaleObservationWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ObservationCanvas"));
		WidgetTree->RootWidget = Canvas;
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
		Size->SetWidthOverride(260.0f);
		UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(Size);
		CanvasSlot->SetAnchors(FAnchors(1,1));
		CanvasSlot->SetAlignment(FVector2D(1,1));
		CanvasSlot->SetPosition(FVector2D(-24.0f, -24.0f));
		CanvasSlot->SetAutoSize(true);

		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
		Panel->SetPadding(FMargin(20.0f, 16.0f));
		Panel->SetBrushColor(FLinearColor(0.015f, 0.045f, 0.065f, 0.88f));
		Size->SetContent(Panel);
		UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>();
		Panel->SetContent(Lines);

		auto AddText = [this, Lines](FName Name, int32 FontSize, bool bBold, float TopMargin)
		{
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
			Text->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), FontSize));
			Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.96f, 1.0f)));
			Text->SetAutoWrapText(true);
			Lines->AddChildToVerticalBox(Text)->SetPadding(FMargin(0.0f, TopMargin, 0.0f, 0.0f));
			return Text;
		};
		UTextBlock* Controls = AddText(TEXT("ExplorationShortcuts"), 14, false, 0.0f);
		Controls->SetText(LOCTEXT("EssentialShortcuts", "H: 자동복귀\nM: 수동 전환\nTab: 시점 전환"));
		if (bVRPresentation) Size->SetVisibility(ESlateVisibility::Collapsed);
		USizeBox* NoticeSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("EncounterNoticeSize"));
		NoticeSize->SetWidthOverride(330);
		auto* NoticeSlot = Canvas->AddChildToCanvas(NoticeSize);
		NoticeSlot->SetAnchors(FAnchors(1,0)); NoticeSlot->SetAlignment(FVector2D(1,0));
		NoticeSlot->SetPosition(FVector2D(-24,24)); NoticeSlot->SetAutoSize(true);
		NoticePanel = WidgetTree->ConstructWidget<UBorder>();
		NoticePanel->SetPadding(FMargin(18,14)); NoticePanel->SetBrushColor(FLinearColor(.015,.045,.065,.92));
		NoticeSize->SetContent(NoticePanel);
		auto* Notices = WidgetTree->ConstructWidget<UVerticalBox>(); NoticePanel->SetContent(Notices);
		WhaleNotice = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WhaleEncounterNotice"));
		JellyNotice = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("JellyEncounterNotice"));
		WhaleNotice->SetText(LOCTEXT("WhaleEncounter", "고래를 만났어요!\n혹등고래 관찰 구역에 도착했습니다."));
		JellyNotice->SetText(LOCTEXT("JellyEncounter", "해파리 주의!\n해파리가 있는 위험 구역입니다."));
		WhaleNotice->SetColorAndOpacity(FSlateColor(FLinearColor(.5,.92,1)));
		JellyNotice->SetColorAndOpacity(FSlateColor(FLinearColor(1,.7,.3)));
		for(auto* Text : {WhaleNotice.Get(), JellyNotice.Get()})
		{
			Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),16)); Text->SetAutoWrapText(true);
			Notices->AddChildToVerticalBox(Text)->SetPadding(FMargin(0,4));
		}


        ReturnPanel = WidgetTree->ConstructWidget<UBorder>();
        ReturnPanel->SetPadding(FMargin(24,18));
        ReturnPanel->SetBrushColor(FLinearColor(.015,.045,.065,.92));
        auto* ReturnSlot = Canvas->AddChildToCanvas(ReturnPanel);
        ReturnSlot->SetAnchors(FAnchors(.5f,.60f));
        ReturnSlot->SetAlignment(FVector2D(.5f,.5f));
        ReturnSlot->SetAutoSize(true);
        ReturnNotice = WidgetTree->ConstructWidget<UTextBlock>();
        ReturnNotice->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),26));
        ReturnNotice->SetColorAndOpacity(FSlateColor(FLinearColor(.8f,.97f,1)));
        ReturnNotice->SetJustification(ETextJustify::Center);
        ReturnNotice->SetWrapTextAt(570);
        ReturnPanel->SetContent(ReturnNotice);
        ReturnBadge = WidgetTree->ConstructWidget<UTextBlock>();
        ReturnBadge->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),22));
        ReturnBadge->SetColorAndOpacity(FSlateColor(FLinearColor(.5f,.92f,1)));
        ReturnBadge->SetShadowOffset(FVector2D(1,2));
        ReturnBadge->SetShadowColorAndOpacity(FLinearColor::Black);
        auto* BadgeSlot = Canvas->AddChildToCanvas(ReturnBadge);
        BadgeSlot->SetAnchors(FAnchors(.76f,.22f));
        BadgeSlot->SetAlignment(FVector2D(.5f,.5f));
        BadgeSlot->SetAutoSize(true);

	}
	ApplyObservationState(State);
	return Super::RebuildWidget();
}

void UWhaleObservationWidget::ApplyObservationState(const FWhaleObservationSnapshot& NewState)
{
	if(NewState.bAvailable && NewState.bInObservationZone && !State.bInObservationZone) WhaleSeconds = 5;
	if(!NewState.bAvailable) WhaleSeconds = 0;
	State = NewState;
	RefreshVisibility();
}

void UWhaleObservationWidget::ApplyReturnState(const FExplorationReturnSnapshot& NewState)
{
	if((NewState.bInsideDangerZone && !ReturnState.bInsideDangerZone)
		|| NewState.DangerEncounterSerial != ReturnState.DangerEncounterSerial) JellySeconds = 5;
    if (NewState.Mode != ReturnState.Mode)
    {
        const auto Previous = ReturnState.Mode;
        if (NewState.Mode == EExplorationReturnMode::PreparingReturn || NewState.Mode == EExplorationReturnMode::Planning)
            ReturnMessage = LOCTEXT("ReturnPlanning", "자동복귀 경로를 준비합니다");
        else if (NewState.Mode == EExplorationReturnMode::Returning)
            ReturnMessage = LOCTEXT("ReturnStarted", "자동복귀를 시작합니다\n탐사선이 출발 지점으로 이동합니다.");
        else if (NewState.Mode == EExplorationReturnMode::Arrived)
            ReturnMessage = LOCTEXT("ReturnArrived", "출발 지점에 도착했습니다");
        else if (NewState.Mode == EExplorationReturnMode::Stopped)
            ReturnMessage = FText::Format(LOCTEXT("ReturnStopped", "탐사가 정지했습니다\n{0}"), NewState.Detail);
        else if (NewState.Mode == EExplorationReturnMode::Manual &&
            (Previous == EExplorationReturnMode::Returning || Previous == EExplorationReturnMode::Planning || Previous == EExplorationReturnMode::PreparingReturn))
            ReturnMessage = LOCTEXT("ReturnManual", "자동복귀가 중단되었습니다\n수동 탐사 모드입니다.");
        else ReturnMessage = FText::GetEmpty();
        ReturnSeconds = ReturnMessage.IsEmpty() ? 0 : 3;
    }
    ReturnState = NewState;
	RefreshVisibility();
}

void UWhaleObservationWidget::RefreshVisibility()
{
    const bool bReturning = ReturnState.Mode == EExplorationReturnMode::Returning;
    const bool bPreparing = ReturnState.Mode == EExplorationReturnMode::Planning || ReturnState.Mode == EExplorationReturnMode::PreparingReturn;
    if (ReturnNotice) ReturnNotice->SetText(ReturnMessage);
    if (ReturnPanel) ReturnPanel->SetVisibility(ReturnSeconds > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    if (ReturnBadge)
    {
        ReturnBadge->SetText(bReturning ? LOCTEXT("ReturnBadge", "자동복귀 중") : LOCTEXT("PlanningBadge", "복귀 경로 준비 중"));
        ReturnBadge->SetVisibility((bReturning || bPreparing) && ReturnSeconds <= 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
	if(WhaleNotice) WhaleNotice->SetVisibility(WhaleSeconds>0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if(JellyNotice) JellyNotice->SetVisibility(JellySeconds>0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if(NoticePanel) NoticePanel->SetVisibility(WhaleSeconds>0 || JellySeconds>0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	// HitTestInvisible also excludes children from driving and mouse input.
	const bool bReturnAvailable = ReturnState.Mode != EExplorationReturnMode::Unavailable;
	SetVisibility(WhaleSeconds>0 || JellySeconds>0 || State.bAvailable || bReturnAvailable || ReturnState.bInsideDangerZone
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UWhaleObservationWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
 Super::NativeTick(Geometry,DeltaTime);
 ReturnSeconds=FMath::Max(0.0f,ReturnSeconds-DeltaTime);
 WhaleSeconds=FMath::Max(0.0f,WhaleSeconds-DeltaTime); JellySeconds=FMath::Max(0.0f,JellySeconds-DeltaTime);
 RefreshVisibility();
}

void UWhaleObservationWidget::GetVisibleNotices(TArray<FExplorationVisibleNotice>& Notices, FString& Badge) const
{
 Notices.Reset(); Badge.Reset();
 if (ReturnSeconds > 0 && !ReturnMessage.IsEmpty()) Notices.Add({TEXT("return"), ReturnMessage.ToString(), ReturnSeconds});
 if (WhaleSeconds > 0 && WhaleNotice) Notices.Add({TEXT("whale"), WhaleNotice->GetText().ToString(), WhaleSeconds});
 if (JellySeconds > 0 && JellyNotice) Notices.Add({TEXT("danger"), JellyNotice->GetText().ToString(), JellySeconds});
 if (ReturnSeconds <= 0 && ReturnBadge && (ReturnState.Mode == EExplorationReturnMode::Returning || ReturnState.Mode == EExplorationReturnMode::Planning || ReturnState.Mode == EExplorationReturnMode::PreparingReturn)) Badge = ReturnBadge->GetText().ToString();
}

#undef LOCTEXT_NAMESPACE
