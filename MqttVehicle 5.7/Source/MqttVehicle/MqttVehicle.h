// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogMqttVehicle, Log, All);
#include "Misc/App.h"

namespace MqttVehicleControl
{
 // OpenXR owns focus while a VR session is active; desktop focus is only the fallback.
 inline bool HasFocus(bool bDesktopFocus)
 {
  return FApp::UseVRFocus() ? FApp::HasVRFocus() : bDesktopFocus;
 }
}
