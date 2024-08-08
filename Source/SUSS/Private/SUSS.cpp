// Copyright Epic Games, Inc. All Rights Reserved.

#include "SUSS.h"

#include "Debug\GameplayDebuggerCategorySUSS.h"
#include "GameplayDebugger.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif
#include "SussSettings.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FSUSSModule"

void FSUSSModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin
	// file per-module

#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "SUSS",
			LOCTEXT("SUSSSettingsName", "SUSS"),
			LOCTEXT("SUSSSettingsDescription", "Configure the Steve's UtilityAI SubSystem"),
			GetMutableDefault<USussSettings>()
		);
	}
#endif

#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("SUSS", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_SUSS::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGame, 2);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif // WITH_GAMEPLAY_DEBUGGER
	
}

void FSUSSModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("SUSS");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif // WITH_GAMEPLAY_DEBUGGER
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSUSSModule, SUSS)
