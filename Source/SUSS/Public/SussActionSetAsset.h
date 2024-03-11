// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SussConsideration.h"
#include "SussAction.h"
#include "SussActionSetAsset.generated.h"


USTRUCT()
struct FSussQuery
{
	GENERATED_BODY()

	/// The tag of the query we want to run
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Suss.Query"))
	FGameplayTag QueryTag;

	/// Parameters to pass to the query
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FSussParameter> Params;

};

USTRUCT()
struct FSussActionDef
{
	GENERATED_BODY()
public:
	/// The action which is going to be called
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USussAction> ActionClass;

	/// Queries to run to provide values for the Considerations. Beware: multiple queries that return different
	/// information will geometrically multiply the number of variations that will need to be considered. E.g. if you
	/// run a query for targets, and for locations, the number of variations will be the number of results from each
	/// multiplied together.
	/// You can have multiple queries for the same piece of context (e.g. 2 target queries) and the results will be combined.
	/// If there are no queries, then the considerations will be executed only once with the "Self" context reference only
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussQuery> Queries;

	/// Considerations score the action and will be run as many times as needed by the combination of results from the queries
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussConsideration> Considerations;

	/// Re-weighting applied to the final result of considerations, should you wish to adjust the final value
	UPROPERTY(EditDefaultsOnly)
	float Weight = 1;

	/// Priority group of this action. Actions will be considered in groups according to their priority, with
	/// higher priority (e.g. 20 is higher priority than 50) actions being evaluated first. If any action in a high
	/// priority group scores > 0, that action will be used even if there is potentially a higher scoring action in a
	/// lower priority group. As soon as one action in a priority group scores > 0 then only actions from that group
	/// will be considered for execution.
	UPROPERTY(EditDefaultsOnly)
	int Priority = 100;

	/// The tags required on the agent for this action to be enabled at all
	/// You can have tag tests in considerations as well, but this is a simple way to completely exclude an
	/// action unless ALL these tags are present
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer RequiredTags;

	/// The tags on the agent which stop this action being available at all
	/// You can have tag tests in considerations as well, but this is a simple way to completely exclude an
	/// action if ANY of these tags are present
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer BlockingTags;

	/// Once the decision has been made to perform this action, what additional "Inertia" score to add to it
	/// to avoid the brain flip/flopping on a boundary condition. This inertia cools down over a period of time so eventually
	/// a better scored decision can interrupt it, if the ActionClass allows interruptions.
	UPROPERTY(EditDefaultsOnly)
	float Inertia = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	float InertiaCooldown = 5.0f;
	
};
/**
 * An action set is a re-usable collection of actions, to make it quicker & easier to build AIs from pre-built behaviours
 */
UCLASS()
class SUSS_API USussActionSetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	/// The action definitions
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussActionDef> ActionDefs;

public:
	TArray<FSussActionDef>& GetActions() { return ActionDefs; }

	
};
