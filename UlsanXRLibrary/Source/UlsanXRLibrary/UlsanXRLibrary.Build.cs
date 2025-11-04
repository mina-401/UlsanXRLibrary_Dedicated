// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UlsanXRLibrary : ModuleRules
{
    public UlsanXRLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
              new string[] {
        ModuleDirectory,  // 기존 모듈 디렉터리 포함
        System.IO.Path.Combine(ModuleDirectory, "Global") // Global 폴더 추가
              }
          );

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine",
            "InputCore", "EnhancedInput",
            "UMG",
            "Sockets", "Networking","OnlineSubsystem","OnlineSubsystemEOS","OnlineSubsystemUtils","VoiceChat",
        });


    }
}
