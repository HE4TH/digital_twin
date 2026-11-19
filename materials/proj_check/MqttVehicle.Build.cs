// Copyright (c) 2026. MQTT Vehicle Bridge - Windows target.

using UnrealBuildTool;

public class MqttVehicle : ModuleRules
{
	public MqttVehicle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Json",
			"JsonUtilities",
			"PhysicsCore",
			"ChaosVehicles",
			"MQTTCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
