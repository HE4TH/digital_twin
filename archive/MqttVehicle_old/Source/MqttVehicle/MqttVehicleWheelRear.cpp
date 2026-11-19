// Copyright Epic Games, Inc. All Rights Reserved.

#include "MqttVehicleWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UMqttVehicleWheelRear::UMqttVehicleWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}