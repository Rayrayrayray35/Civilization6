// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
public class civi : ModuleRules
{
    public civi(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 添加 "UMG" 到 PublicDependencyModuleNames
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

        // 添加 Slate 模块
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
    }
}
