#include "SubmersibleViewWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"


USubmersibleViewWidget::USubmersibleViewWidget(const FObjectInitializer& Initializer) : Super(Initializer)
{
	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> USubmersibleViewWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	if (!WidgetTree->RootWidget)
	{
		auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Canvas;
		auto* Frame = WidgetTree->ConstructWidget<UBorder>();
		Frame->SetBrushColor(FLinearColor(0.012f,0.04f,0.055f,0.96f));
		Frame->SetPadding(FMargin(6));
		auto* CanvasSlot = Canvas->AddChildToCanvas(Frame);
		CanvasSlot->SetAnchors(FAnchors(0,1));
		CanvasSlot->SetAlignment(FVector2D(0,1));
		CanvasSlot->SetPosition(FVector2D(24,-24));
		CanvasSlot->SetAutoSize(true);
		auto* Size = WidgetTree->ConstructWidget<USizeBox>();
		Size->SetWidthOverride(512); Size->SetHeightOverride(288);
		Frame->SetContent(Size);
		Feed = WidgetTree->ConstructWidget<UImage>();
		Size->SetContent(Feed);
	}
	return Super::RebuildWidget();
}

void USubmersibleViewWidget::SetFeed(UMaterialInterface* Material)
{
	if (Feed) Feed->SetBrushFromMaterial(Material);
}
