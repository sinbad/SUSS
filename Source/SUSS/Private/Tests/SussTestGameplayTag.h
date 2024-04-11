#pragma once

#include "NativeGameplayTags.h"

class FSussTempNativeGameplayTag : public FNativeGameplayTag
{
public:
	// We need this empty constructor for generated code to be happy
	FSussTempNativeGameplayTag() : FNativeGameplayTag(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   "Suss.Temp",
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD){}

	FSussTempNativeGameplayTag(FName PluginName, FName ModuleName, FName TagName, const FString& TagDevComment, ENativeGameplayTagToken Tok)
		: FNativeGameplayTag(PluginName, ModuleName, TagName, TagDevComment, Tok) {}
};