// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SussConsideration.h"
#include "SussActionSet.generated.h"

USTRUCT()
struct FSussActionDef
{
	GENERATED_BODY()

	// The action which is going to be called
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USussAction> ActionClass;

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

};
/**
 * An action set is a re-usable collection of actions, to make it quicker & easier to build AIs from pre-built behaviours
 */
UCLASS()
class SUSS_API USussActionSet : public UDataAsset
{
	GENERATED_BODY()

protected:
	/// The action definitions
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussActionDef> ActionDefs;

	
};
