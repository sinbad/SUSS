
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
class USussTestNamedLocationValueQueryProvider : public USussNamedValueQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestNamedLocationValueQueryProvider()
	: TAG_TEMP(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   TagName,
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
		QueryValueName = FName("MapRef");
		QueryValueType = ESussContextValueType::Vector;
	}
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	virtual void ExecuteQuery(USussBrainComponent* Brain,
	                          AActor* Self,
	                          const TMap<FName, FSussParameter>& Params,
	                          TArray<FSussContextValue>& OutResults) override
	{
		OutResults.Add(FVector(120, -450, 80));
		OutResults.Add(FVector(70, 123, -210));
		OutResults.Add(FVector(-35, 65, 0));
	}
};

UCLASS()
class USussTestNamedFloatValueQueryProvider : public USussNamedValueQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestNamedFloatValueQueryProvider()
	: TAG_TEMP(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   TagName,
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
		QueryValueName = FName("Range");
		QueryValueType = ESussContextValueType::Float;

	}
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	virtual void ExecuteQuery(USussBrainComponent* Brain,
							  AActor* Self,
							  const TMap<FName, FSussParameter>& Params,
							  TArray<FSussContextValue>& OutResults) override
	{
		// The .f is very important so this does become a float value not an int value
		// Could have used AddValueFloat for clarity
		OutResults.Add(2000.0f);
		OutResults.Add(5000.0f);
	}
};

USTRUCT()
struct FSussTestContextValueStruct : public FSussContextValueStructBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int IntValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatValue;

	FSussTestContextValueStruct(): IntValue(0), FloatValue(0)
	{
	}

	FSussTestContextValueStruct(int i, float f) : IntValue(i), FloatValue(f) {}
};

UCLASS()
class USussTestNamedStructSharedValueQueryProvider : public USussNamedValueQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestNamedStructSharedValueQueryProvider()
	: TAG_TEMP(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   TagName,
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
		QueryValueName = FName("Struct");
		QueryValueType = ESussContextValueType::Struct;
	}
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	virtual void ExecuteQuery(USussBrainComponent* Brain,
							  AActor* Self,
							  const TMap<FName, FSussParameter>& Params,
							  TArray<FSussContextValue>& OutResults) override
	{
		AddValueStruct(MakeShareable(new FSussTestContextValueStruct(200, 123.4f)));
		AddValueStruct(MakeShareable(new FSussTestContextValueStruct(-30, 785.2f)));
	}
};

UCLASS()
class USussTestNamedStructRawPointerQueryProvider : public USussNamedValueQueryProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;

	USussTestNamedStructRawPointerQueryProvider()
	: TAG_TEMP(UE_PLUGIN_NAME,
		   UE_MODULE_NAME,
		   TagName,
		   TEXT(""),
		   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		QueryTag = TAG_TEMP;
		QueryValueName = FName("Struct");
		QueryValueType = ESussContextValueType::Struct;

		// This is generally a good idea when using the raw pointer version so we don't cache raw pointers to things
		// that could go away at any time
		bUseCachedResults = false;

		ArrayOfStructs.Add(FSussTestContextValueStruct(200, 123.4f));
		ArrayOfStructs.Add(FSussTestContextValueStruct(-30, 785.2f));
	}
protected:
	// Define this locally so that it is destroyed after test finishes & doesn't show up in tag browser
	FSussTempNativeGameplayTag TAG_TEMP;

	TArray<FSussTestContextValueStruct> ArrayOfStructs;
	
	virtual void ExecuteQuery(USussBrainComponent* Brain,
							  AActor* Self,
							  const TMap<FName, FSussParameter>& Params,
							  TArray<FSussContextValue>& OutResults) override
	{
		// We're going to use some structs that are NOT passed by shared pointer but are kept on this class
		// In real cases these structs would probably be on other classes (hence the lack of cacheing)
		AddValueStruct(&ArrayOfStructs[0]);
		AddValueStruct(&ArrayOfStructs[1]);
	}
};


inline void RegisterTestQueryProviders(UWorld* World)
{
	if (auto SUSS = GetSUSS(World))
	{
		SUSS->RegisterQueryProviderClass(USussTestSingleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestMultipleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestNamedLocationValueQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestNamedFloatValueQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestNamedStructSharedValueQueryProvider::StaticClass());
		SUSS->RegisterQueryProviderClass(USussTestNamedStructRawPointerQueryProvider::StaticClass());
	}
}

inline void UnregisterTestQueryProviders(UWorld* World)
{
	if (auto SUSS = GetSUSS(World))
	{
		SUSS->UnregisterQueryProviderClass(USussTestSingleLocationQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestMultipleLocationQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestNamedLocationValueQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestNamedFloatValueQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestNamedStructSharedValueQueryProvider::StaticClass());
		SUSS->UnregisterQueryProviderClass(USussTestNamedStructRawPointerQueryProvider::StaticClass());
	}
}
