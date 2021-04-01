// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class VRClassroom : ModuleRules
{
	public VRClassroom(ReadOnlyTargetRules ROTargetRules) : base(ROTargetRules)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core",
            "CoreUObject",
            "Engine",
            "AIModule",
            "Http",
            "Json",
            "JsonUtilities",
            "InputCore",
            "GameplayAbilities"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { "UMG" });

        //PublicIncludePathModuleNames.AddRange(new string[] { "OWSPlugin" });

        // Uncomment if you are using Slate UI
        //PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
        //Definitions.Add("AES_KEY=FF82B5451E21C090AE76622AAF24BCB0");
    }
}
