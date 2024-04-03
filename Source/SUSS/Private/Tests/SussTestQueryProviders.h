
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussGameSubsystem.h"
#include "SussQueryProvider.h"
#include "SussTestQueryProviders.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TestSingleLocationQuery);
UCLASS()
class USussTestSingleLocationQueryProvider : public USussLocationQueryProvider
{
	GENERATED_BODY()
public:
	USussTestSingleLocationQueryProvider()
	{
		QueryTag = TAG_TestSingleLocationQuery;
	}

	int NumTimesRun = 0;
protected:

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

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TestMultipleLocationQuery);
UCLASS()
class USussTestMultipleLocationQueryProvider : public USussLocationQueryProvider
{
	GENERATED_BODY()
public:
	USussTestMultipleLocationQueryProvider() { QueryTag = TAG_TestMultipleLocationQuery; }
protected:
	
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

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TestMultipleRotationQuery);
UCLASS()
class USussTestMultipleRotationQueryProvider : public USussRotationQueryProvider
{
	GENERATED_BODY()
public:
	USussTestMultipleRotationQueryProvider() { QueryTag = TAG_TestMultipleRotationQuery; }
protected:

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
