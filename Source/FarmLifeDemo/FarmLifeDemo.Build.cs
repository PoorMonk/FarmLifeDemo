// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FarmLifeDemo : ModuleRules
{
	public FarmLifeDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
