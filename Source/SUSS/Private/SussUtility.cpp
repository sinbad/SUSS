#include "SussUtility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SussSettings.h"


bool USussUtility::IsActionEnabled(TSubclassOf<USussAction> ActionClass)
{
	if (auto Settings = GetDefault<USussSettings>())
	{
		return !Settings->DisabledActions.Contains(ActionClass);
	}

	return true;
}

bool USussUtility::ActorHasTag(AActor* Actor, const FGameplayTag& Tag)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasMatchingGameplayTag(Tag);
	}

	return false;
}

bool USussUtility::ActorHasAnyTags(AActor* Actor, const FGameplayTagContainer& Tags)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasAnyMatchingGameplayTags(Tags);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasAnyMatchingGameplayTags(Tags);
	}

	return false;
}

bool USussUtility::ActorHasAllTags(AActor* Actor, const FGameplayTagContainer& Tags)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasAllMatchingGameplayTags(Tags);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasAllMatchingGameplayTags(Tags);
	}

	return false;
}
