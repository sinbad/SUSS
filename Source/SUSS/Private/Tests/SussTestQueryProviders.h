
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
	USussTestSingleLocationQueryProvider() { QueryTag = TAG_TestSingleLocationQuery; }
protected:
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params) override
	{
		CachedResults.Reset();
		CachedResults.Add(FVector(10, -20, 50));
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
		const TMap<FName, FSussParameter>& Params) override
	{
		CachedResults.Reset();
		CachedResults.Add(FVector(10, -20, 50));
		CachedResults.Add(FVector(20, 100, -2));
		CachedResults.Add(FVector(-40, 220, 750));
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
		const TMap<FName, FSussParameter>& Params) override
	{
		CachedResults.Reset();
		CachedResults.Add(FRotator(10, -20, 50));
		CachedResults.Add(FRotator(20, 100, -2));
		CachedResults.Add(FRotator(-40, 220, 750));
	}

};

inline void RegisterTestQueryProviders(UWorld* World)
{
	if (auto SUSS = GetSUSS(World))
	{
		SUSS->RegisterQueryProvider(USussTestSingleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProvider(USussTestMultipleLocationQueryProvider::StaticClass());
		SUSS->RegisterQueryProvider(USussTestMultipleRotationQueryProvider::StaticClass());
	}
}
