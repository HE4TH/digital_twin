// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MqttVehiclePawn.h"
#include "MqttVehicleSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class AMqttVehicleSportsCar : public AMqttVehiclePawn
{
	GENERATED_BODY()
	
public:

	AMqttVehicleSportsCar();
};
