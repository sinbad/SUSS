
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussGameSubsystem.h"
#include "SussQueryProvider.h"
#include "SussTestGameplayTag.h"
#include "SussTestQueryProviders.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN();
UCLASS()
class USussTestSingleLocationQueryProvider : public USussLocationQueryProvider
{
	GENERATED_BODY()
public:

	static const FName TagName;

	USussTestSingleLocationQueryProvider()
		: TAG_TEMP(UE_PLUGIN_NAME,
		           UE_MODULE_NAME,
		           TagName,
		           TEXT(""),
		           ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
	}

	int NumTimesRun = 0;
protected:

	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<FVector>& OutResults) override
	{
		float X = 10;
		float Y = -20;
		if (auto pVal = Params.Find("OverrideX"))
		{
			X = pVal->FloatValue;
		}
		if (auto pVal = Params.Find("OverrideY"))
		{
			Y = pVal->FloatValue;
		}

		OutResults.Add(FVector(X, Y, 50));
		

		++NumTimesRun;
	}
};

UCLASS()
class USussTestMultipleLocationQueryProvider : public USussLocationQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestMultipleLocationQueryProvider()
		: TAG_TEMP(UE_PLUGIN_NAME,
		           UE_MODULE_NAME,
		           TagName,
		           TEXT(""),
		           ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
	}
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;
	
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<FVector>& OutResults) override
	{
		OutResults.Add(FVector(10, -20, 50));
		OutResults.Add(FVector(20, 100, -2));
		OutResults.Add(FVector(-40, 220, 750));
	}
};

UCLASS()
class USussTestMultipleRotationQueryProvider : public USussRotationQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestMultipleRotationQueryProvider()
	: TAG_TEMP(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   TagName,
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{ QueryTag = TAG_TEMP; }
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<FRotator>& OutResults) override
	{
		OutResults.Add(FRotator(10, -20, 50));
		OutResults.Add(FRotator(20, 100, -2));
		OutResults.Add(FRotator(-40, 220, 750));
	}
};

inline void RegisterTestQueryProviders(UWorld* World)
{
	if (auto SUSS = GetSUSS(World))
	{
		SUSS->RegisterQueryProviderClass(USussTestSingleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestMultipleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestMultipleRotationQueryProvider::StaticClass());
	}
}

inline void UnregisterTestQueryProviders(UWorld* World)
{
	if (auto SUSS = GetSUSS(World))
	{
		SUSS->UnregisterQueryProviderClass(USussTestSingleLocationQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestMultipleLocationQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestMultipleRotationQueryProvider::StaticClass());
	}
}
