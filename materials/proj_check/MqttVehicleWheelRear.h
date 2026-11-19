// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "MqttVehicleWheelRear.generated.h"

/**
 *  Base rear wheel definition.
 */
UCLASS()
class UMqttVehicleWheelRear : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	UMqttVehicleWheelRear();
};
