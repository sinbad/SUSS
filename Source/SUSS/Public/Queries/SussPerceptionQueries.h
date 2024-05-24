// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussQueryProvider.h"
#include "Perception/AIPerceptionTypes.h"
#include "SussPerceptionQueries.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownTargets);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownHostiles);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownNonHostiles);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryPerceptionKnownHostilesExtended);

/**
 * Query provider which provides a list of all targets (hostile, friendly, neutral) that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten.
 * Optional parameters:
 *    "Sense": Identify the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 *    "IgnoreTags": TagContainer of tags which you want to ignore if the actor has (e.g. dead, invisible)
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
		const FSussContext& Context,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};

/**
 * Query provider which provides a list of all hostiles that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten, but may
 * not be *currently* perceived.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 *    "IgnoreTags": TagContainer of tags which you want to ignore if the actor has (e.g. dead, invisible)
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
		const FSussContext& Context,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};

/**
 * Query provider which provides a list of all non-hostiles that are "known" by the perception system of the agent.
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten, but may
 * not be *currently* perceived.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 *    "IgnoreTags": TagContainer of tags which you want to ignore if the actor has (e.g. dead, invisible)
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
		const FSussContext& Context,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override;
};

struct FActorPerceptionInfo;

USTRUCT(BlueprintType)
struct FSussActorPerceptionInfo : public FSussContextValueStructBase
{
	GENERATED_BODY()
public:
	/// The actor that has been sensed (is the instigator of heard things)
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target;

	/// Helper to indicate whether the actor has been seen & not forgotten
	UPROPERTY(BlueprintReadOnly)
	uint32 bIsSeen : 1;

	/// Helper to indicate whether the actor has been heard & not forgotten
	UPROPERTY(BlueprintReadOnly)
	uint32 bIsHeard : 1;

	/// Location that the actor was last seen or heard
	UPROPERTY(BlueprintReadOnly)
	FVector LastLocation;

	/// Full information about the actual senses
	UPROPERTY(BlueprintReadOnly)
	TArray<FAIStimulus> LastSensedStimuli;

	/// Whether the target is hostile
	UPROPERTY(BlueprintReadOnly)
	uint32 bIsHostile : 1;

	FSussActorPerceptionInfo() : Target(NULL), bIsSeen(0), bIsHeard(0), bIsHostile(false)
	{
	}

	FSussActorPerceptionInfo(const FActorPerceptionInfo& Info);
	virtual FString ToString() const override;
};

/**
 * Query provider which provides extended info about all hostile targets that are "known" by the perception
 * system of the agent. Instead of just providing a target, it provides extra information about when & where the
 * target was last sensed, so it returns a named value struct called "PerceptionInfo" of type FSussActorPerceptionInfo, instead of just a target.
 * 
 * Requires that the agent's AI controller has a UAIPerceptionComponent.
 * "Known" means that the targets have been perceived at some point and have not yet been forgotten.
 * Optional parameters:
 *    "Sense": A name parameter identifying the single sense you want to test ("Sight", "Hearing", "Damage", "Touch")
 *    "IgnoreTags": TagContainer of tags which you want to ignore if the actor has (e.g. dead, invisible)
 */
UCLASS()
class SUSS_API USussPerceptionKnownHostilesExtendedQueryProvider : public USussNamedValueQueryProvider
{
	GENERATED_BODY()

public:
	USussPerceptionKnownHostilesExtendedQueryProvider();
protected:
	virtual void ExecuteQuery(USussBrainComponent* Brain,
		AActor* Self,
		const TMap<FName, FSussParameter>& Params,
		const FSussContext& Context,
		TArray<FSussContextValue>& OutResults) override;
	
};