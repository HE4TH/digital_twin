// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "MqttVehicleWheelFront.generated.h"

/**
 *  Base front wheel definition.
 */
UCLASS()
class UMqttVehicleWheelFront : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	UMqttVehicleWheelFront();
};