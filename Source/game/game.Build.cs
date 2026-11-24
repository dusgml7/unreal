// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class game : ModuleRules
{
	public game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		// UI(위젯) 작업용 모듈: UMG = 위젯 블루프린트, Slate/SlateCore = UMG가 내부적으로 쓰는 UI 프레임워크
		PrivateDependencyModuleNames.AddRange(new string[] { "UMG", "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
