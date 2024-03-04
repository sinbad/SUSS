// Copyright Epic Games, Inc. All Rights Reserved.

#include "SUSS.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "SussSettings.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FSUSSModule"

void FSUSSModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

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
}

void FSUSSModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSUSSModule, SUSS)
