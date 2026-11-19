#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubmersibleViewWidget.generated.h"

class UImage;
class UMaterialInterface;

UCLASS()
class MQTTVEHICLE_API USubmersibleViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	USubmersibleViewWidget(const FObjectInitializer& Initializer);
	void SetFeed(UMaterialInterface* Material);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	UPROPERTY(Transient) TObjectPtr<UImage> Feed;
};
