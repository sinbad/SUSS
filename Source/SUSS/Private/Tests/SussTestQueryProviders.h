
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
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

