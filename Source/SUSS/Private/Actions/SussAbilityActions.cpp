#include "Actions/SussAbilityActions.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SussCommon.h"
#include "SussParameter.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussActionActivateAbility, "Suss.Action.Ability.Activate", "Activate a gameplay ability by tag. Requires parameter 'Tag', optional parameters 'CompletionDelay' and 'AllowRemote')")

void USussActivateAbilityByClassAction::PerformAction_Implementation(const FSussContext& Context,
                                                                     const TMap<FName, FSussParameter>& Params,
                                                                     TSubclassOf<USussAction> PreviousActionClass)
{
	if (IsValid(Context.ControlledActor) && (IsValid(AbilityClass) || !AbilityTags.IsEmpty()))
	{
		if (auto ASC
			= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.ControlledActor))
		{
			bool bActivated = IsValid(AbilityClass) ?
				bActivated = ASC->TryActivateAbilityByClass(AbilityClass, bAllowRemoteActivation) : 
				bActivated = ASC->TryActivateAbilitiesByTag(AbilityTags, bAllowRemoteActivation);
			
			if (bActivated)
			{
				// Ability was (probably) activated
				if (CompletionDelay > 0)
				{
					FTimerHandle Temp;
					Context.ControlledActor->GetWorldTimerManager().SetTimer(Temp, this, &USussActivateAbilityByClassAction::DelayedCompletion, CompletionDelay, false);
				}
				else
				{
					ActionCompleted();
				}
				return;
			}
			else
			{
				UE_LOG(LogSuss, Warning, TEXT("Could not activate ability %s on agent %s, may not have ability or costs could not be met"),
					IsValid(AbilityClass) ? *AbilityClass->GetName() : *AbilityTags.ToStringSimple(), *Context.ControlledActor->GetActorNameOrLabel());
			}
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Could not activate ability %s on agent %s, no AbilitySystemComponent"),
				IsValid(AbilityClass) ? *AbilityClass->GetName() : *AbilityTags.ToStringSimple(), *Context.ControlledActor->GetActorNameOrLabel());
		}
	}

	// Fallback for failure
	ActionCompleted();
}

void USussActivateAbilityByClassAction::DelayedCompletion()
{
	ActionCompleted();
}

USussActivateAbilityAction::USussActivateAbilityAction()
{
	ActionTag = TAG_SussActionActivateAbility;
	// By default don't allow interruptions
	bAllowInterruptions = false;
}

void USussActivateAbilityAction::PerformAction_Implementation(const FSussContext& Context,
	const TMap<FName, FSussParameter>& Params,
	TSubclassOf<USussAction> PreviousActionClass)
{
	if (IsValid(Context.ControlledActor))
	{
		if (auto ASC
			= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.ControlledActor))
		{
			if (auto pTagParam = Params.Find(SUSS::TagParamName))
			{
				if (pTagParam->Tag.IsValid())
				{
					// Would we activate any abilities?
					const FGameplayTagContainer Tags(pTagParam->Tag);
					AbilitiesActivating.Empty();
					ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Tags, AbilitiesActivating);

					if (AbilitiesActivating.Num() > 0)
					{
						bool bAllowRemoteActivation = true;
						if (auto pAllowRemoteParam = Params.Find(SUSS::AllowRemoteParamName))
						{
							bAllowRemoteActivation = pAllowRemoteParam->BoolValue;
						}
						PostCompletionDelay = 0;
						if (auto pDelayParam = Params.Find(SUSS::CompletionDelayParamName))
						{
							PostCompletionDelay = pDelayParam->FloatValue;
						}

						// So that we can be called when each ability ends (must be registered before activating)
						OnAbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &USussActivateAbilityAction::OnAbilityEnded);

						if (!ASC->TryActivateAbilitiesByTag(Tags, bAllowRemoteActivation))
						{
							UE_LOG(LogSuss, Warning, TEXT("Could not activate ability %s on agent %s, may not have ability or costs could not be met"),
								*pTagParam->Tag.ToString(), *Context.ControlledActor->GetActorNameOrLabel());
						}
					}	
					
				}
			}
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Could not activate ability on agent %s, no AbilitySystemComponent"),
				*Context.ControlledActor->GetActorNameOrLabel());
		}

	}

}

void USussActivateAbilityAction::AllAbilitiesEnded()
{
	
	if (IsValid(CurrentContext.ControlledActor) && OnAbilityEndedHandle.IsValid())
	{
		if (auto ASC
			= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentContext.ControlledActor))
		{
			ASC->OnAbilityEnded.Remove(OnAbilityEndedHandle);
		}
	}
	OnAbilityEndedHandle.Reset();
	
	if (PostCompletionDelay > 0)
	{
		FTimerHandle Temp;
		CurrentContext.ControlledActor->GetWorldTimerManager().SetTimer(Temp, this, &USussActivateAbilityAction::DelayedCompletion, PostCompletionDelay, false);
	}
	else
	{
		ActionCompleted();
	}
}

void USussActivateAbilityAction::DelayedCompletion()
{
	ActionCompleted();
}

void USussActivateAbilityAction::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
	for (int i = 0; i < AbilitiesActivating.Num(); ++i)
	{
		if (AbilitiesActivating[i]->Handle == EndedData.AbilitySpecHandle)
		{
			AbilitiesActivating.RemoveAt(i);
			break;
		}
	}

	if (AbilitiesActivating.IsEmpty())
	{
		AllAbilitiesEnded();
	}
}

