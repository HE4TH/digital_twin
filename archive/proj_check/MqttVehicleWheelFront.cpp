// Copyright Epic Games, Inc. All Rights Reserved.

#include "MqttVehicleWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UMqttVehicleWheelFront::UMqttVehicleWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}