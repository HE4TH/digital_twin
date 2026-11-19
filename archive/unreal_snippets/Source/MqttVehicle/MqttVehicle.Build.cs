// MqttVehicle.Build.cs
//
// Vehicle 템플릿의 기본 의존성 + MQTT 브릿지에 필요한 것들.
//
// 템플릿은 SportsCar / OffroadCar / Variant_OffRoad / Variant_TimeTrial 처럼
// 소스를 하위 폴더로 나눠두고, 서로를 폴더 없이 include 한다.
//   예)  #include "MqttVehiclePawn.h"
// 그래서 모듈 폴더 아래의 모든 하위 폴더를 include 경로에 넣어줘야 한다.
// 폴더 목록을 손으로 적으면 템플릿 구조가 바뀔 때마다 깨지므로 자동으로 훑는다.

using System.IO;
using UnrealBuildTool;

public class MqttVehicle : ModuleRules
{
	public MqttVehicle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			// --- Vehicle 템플릿이 쓰는 것들 ---
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"PhysicsCore",
			"ChaosVehicles",
			"UMG",
			"Slate",
			"SlateCore",

			// --- MQTT 브릿지가 쓰는 것들 ---
			"Json",
			"JsonUtilities",
			"MQTTCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// 모듈 폴더 자신과 모든 하위 폴더를 include 경로에 추가한다.
		PublicIncludePaths.Add(ModuleDirectory);
		foreach (string SubDir in Directory.GetDirectories(ModuleDirectory, "*", SearchOption.AllDirectories))
		{
			PublicIncludePaths.Add(SubDir);
		}
	}
}
