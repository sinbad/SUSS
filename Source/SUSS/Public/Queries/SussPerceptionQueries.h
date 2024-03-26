// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussQueryProvider.h"
#include "SussPerceptionQueries.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownTargets);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownHostiles);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownNonHostiles);

/**
 * Query provider which provides a list of all targets (hostile, friendly, neutral) that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 */
UCLASS()
class SUSS_API USussPerceptionKnownTargetsQueryProvider : public USussTargetQueryProvider
{
	GENERATED_BODY()

public:
	USussPerceptionKnownTargetsQueryProvider();
protected:
	
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};

/**
 * Query provider which provides a list of all hostiles that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten, but may
 * not be *currently* perceived.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 */
UCLASS()
class SUSS_API USussPerceptionKnownHostilesQueryProvider : public USussTargetQueryProvider
{
	GENERATED_BODY()
public:
	USussPerceptionKnownHostilesQueryProvider();
protected:
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};

/**
 * Query provider which provides a list of all non-hostiles that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten, but may
 * not be *currently* perceived.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 */
UCLASS()
class SUSS_API USussPerceptionKnownNonHostilesQueryProvider : public USussTargetQueryProvider
{
	GENERATED_BODY()
public:
	USussPerceptionKnownNonHostilesQueryProvider();
protected:
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};
