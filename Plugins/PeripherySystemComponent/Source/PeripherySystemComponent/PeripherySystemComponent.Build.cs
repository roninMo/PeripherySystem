// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PeripherySystemComponent : ModuleRules
{
	public PeripherySystemComponent(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"NetCore",
				"PhysicsCore",
				"DataRegistry"
			}
		);

	}
}
