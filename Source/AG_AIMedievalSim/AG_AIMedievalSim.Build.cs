// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AG_AIMedievalSim : ModuleRules
{
	public AG_AIMedievalSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "AIModule", "GameplayTasks", "UMG", "Slate", "SlateCore", "CustomMovementPlugin", "AlesAndFables" });


    }
}
