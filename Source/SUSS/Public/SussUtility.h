// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SussUtility.generated.h"

/**
 * 
 */
UCLASS()
class SUSS_API USussUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/// Global config can enable / disable just certain actions, so check this
	UFUNCTION(BlueprintCallable)
	static bool IsActionEnabled(TSubclassOf<USussAction> ActionClass);
	/// Check whether an actor has a specific tag
	static bool ActorHasTag(AActor* Actor, const FGameplayTag& Tag);
	/// Check whether an actor has ANY of the supplied tags
	static bool ActorHasAnyTags(AActor* Actor, const FGameplayTagContainer& Tags);
	/// Check whether an actor has ALL of the supplied tags
	static bool ActorHasAllTags(AActor* Actor, const FGameplayTagContainer& Tags);

};
