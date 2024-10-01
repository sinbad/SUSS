#include "Actions/SussAbilityActions.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SussCommon.h"
#include "SussParameter.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussActionActivateAbility, "Suss.Action.Ability.Activate", "Activate a gameplay ability by tag. Requires parameter 'Tag', optional parameters 'WaitForEnd', 'CompletionDelay' and 'AllowRemote')")

void USussActivateAbilityActionBase::ActivateWithParams(const FSussContext& Context, const TMap<FName, FSussParameter>& Params)
{
	bool bAllowRemoteActivation = true;
	bool bWaitForEnd = true;
	float Delay = 0;

	// Apply general params
	if (auto pAllowRemoteParam = Params.Find(SUSS::AllowRemoteParamName))
	{
		bAllowRemoteActivation = pAllowRemoteParam->BoolValue;
	}
	if (auto pWaitParam = Params.Find(SUSS::WaitForEndParamName))
	{
		bWaitForEnd = pWaitParam->BoolValue;
	}

	if (auto pDelayParam = Params.Find(SUSS::CompletionDelayParamName))
	{
		Delay = pDelayParam->FloatValue;
	}

	Activate(Context, Delay, bAllowRemoteActivation, bWaitForEnd);
}

void USussActivateAbilityActionBase::Activate(const FSussContext& Context, float Delay, bool bAllowRemote, bool bWaitForEnd)
{
	PostCompletionDelay = Delay;
	if (AbilitiesActivating.IsEmpty())
	{
		CompleteWithMaybeDelay();
		return;
	}

	bool bSuccess = false;
	if (IsValid(Context.ControlledActor))
	{
		if (auto ASC
			= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.ControlledActor))
		{

			if (bWaitForEnd)
			{
				// So that we can be called when each ability ends (must be registered before activating)
				OnAbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &USussActivateAbilityAction::OnAbilityEnded);
			}

			for (int i = 0; i < AbilitiesActivating.Num(); ++i)
			{
				auto Spec = AbilitiesActivating[i];
				if (ASC->TryActivateAbility(Spec->Handle, bAllowRemote))
				{
					bSuccess = true;
				}
				else
				{
					AbilitiesActivating.RemoveAt(i);
					--i;
				}
			}
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Could not activate ability on agent %s, no AbilitySystemComponent"),
				*Context.ControlledActor->GetActorNameOrLabel());
		}

	}

	// Immediate completion on failure or if not waiting (but still respect delay)
	if (!bSuccess || !bWaitForEnd)
	{
		CompleteWithMaybeDelay();
	}

}


void USussActivateAbilityActionBase::Reset_Implementation()
{
	Super::Reset_Implementation();

	PostCompletionDelay = 0;
}

void USussActivateAbilityActionBase::DelayedCompletion()
{
	ActionCompleted();
}


void USussActivateAbilityActionBase::CompleteWithMaybeDelay()
{
	
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


void USussActivateAbilityActionBase::OnAbilityEnded(const FAbilityEndedData& EndedData)
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
		if (IsValid(CurrentContext.ControlledActor) && OnAbilityEndedHandle.IsValid())
		{
			if (auto ASC
				= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentContext.ControlledActor))
			{
				ASC->OnAbilityEnded.Remove(OnAbilityEndedHandle);
			}
		}
		OnAbilityEndedHandle.Reset();
		
		CompleteWithMaybeDelay();
	}
}


void USussActivateAbilityByClassAction::PerformAction_Implementation(const FSussContext& Context,
                                                                     const TMap<FName, FSussParameter>& Params,
                                                                     TSubclassOf<USussAction> PreviousActionClass)
{
	bool bSuccess = false;
	if (IsValid(Context.ControlledActor) && (IsValid(AbilityClass) || !AbilityTags.IsEmpty()))
	{
		if (auto ASC
			= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.ControlledActor))
		{
			AbilitiesActivating.Empty();
			if (IsValid(AbilityClass))
			{
				auto AbilityCDO = AbilityClass.GetDefaultObject();
				for (auto& Spec : ASC->GetActivatableAbilities())
				{
					if (Spec.Ability == AbilityCDO)
					{
						AbilitiesActivating.Add(&Spec);
						break;
					}
				}
			}
			else
			{
				ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags, AbilitiesActivating);
			}

			Activate(Context, CompletionDelay, bAllowRemoteActivation, bWaitForAbilityEnd);
			bSuccess = true;
			
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Could not activate ability %s on agent %s, no AbilitySystemComponent"),
				IsValid(AbilityClass) ? *AbilityClass->GetName() : *AbilityTags.ToStringSimple(), *Context.ControlledActor->GetActorNameOrLabel());
		}
	}

	if (!bSuccess)
	{
		ActionCompleted();
	}
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
	bool bSuccess = false;
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
					
					ActivateWithParams(Context, Params);
					bSuccess = true;
					
				}
			}
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Could not activate ability on agent %s, no AbilitySystemComponent"),
				*Context.ControlledActor->GetActorNameOrLabel());
		}

	}

	if (!bSuccess)
	{
		ActionCompleted();
	}

}
