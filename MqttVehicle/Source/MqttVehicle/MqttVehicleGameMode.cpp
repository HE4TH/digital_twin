// Copyright Epic Games, Inc. All Rights Reserved.

#include "MqttVehicleGameMode.h"
#include "MqttVehiclePlayerController.h"

AMqttVehicleGameMode::AMqttVehicleGameMode()
{
	PlayerControllerClass = AMqttVehiclePlayerController::StaticClass();
}
