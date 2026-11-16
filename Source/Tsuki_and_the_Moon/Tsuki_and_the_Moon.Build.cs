// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tsuki_and_the_Moon : ModuleRules
{
	public Tsuki_and_the_Moon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets sources include by folder, e.g. "Properties/PropertyHolderComponent.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "DeveloperSettings", "UMG", "Slate", "SlateCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
