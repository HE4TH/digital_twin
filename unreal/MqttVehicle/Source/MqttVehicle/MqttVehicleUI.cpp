// Copyright Epic Games, Inc. All Rights Reserved.


#include "MqttVehicleUI.h"
#include "Components/TextBlock.h"

void UMqttVehicleUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	const float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 1.0f / 44.704f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
	// The template Blueprint's unit text can disagree with its bIsMPH setting.
	// Keep the existing UnitLabel tied to the same setting as the numeric value.
	if (UTextBlock* UnitLabel = Cast<UTextBlock>(GetWidgetFromName(TEXT("UnitLabel"))))
	{
		const FText Unit = FText::FromString(bIsMPH ? TEXT("MPH") : TEXT("km/h"));
		if (!UnitLabel->GetText().EqualTo(Unit)) UnitLabel->SetText(Unit);
	}
}

void UMqttVehicleUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}
