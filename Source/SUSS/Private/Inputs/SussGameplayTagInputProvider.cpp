
#include "Inputs/SussGameplayTagInputProvider.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"


float USussGameplayTagInputProvider::ScoreTagsOnActor(const AActor* FromActor) const
{
	if (IsValid(FromActor))
	{
		if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(FromActor))
		{
			float Score = 0;

			for (auto& Tag : PositiveScoreTags)
			{
				const int Count = ASC->GetTagCount(Tag);
				if (Count > 0)
				{
					Score += bTagCountAffectsScore ? Count * PositiveScoreValue : PositiveScoreValue;
				}
			}
			for (auto& Tag : NegativeScoreTags)
			{
				const int Count = ASC->GetTagCount(Tag);
				if (Count > 0)
				{
					Score -= bTagCountAffectsScore ? Count * NegativeScoreValue : NegativeScoreValue;
				}
			}

			if (Divisor > 0)
			{
				Score /= Divisor;
			}

			if (bClampResult)
			{
				Score = FMath::Clamp(Score, 0.0f, 1.0f);
			}

			return Score;
		}
	}

	return 0;
}

float USussGameplayTagSelfInputProvider::Evaluate_Implementation(
	const class USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	return ScoreTagsOnActor(Context.ControlledActor);
}

float USussGameplayTagTargetInputProvider::Evaluate_Implementation(
	const class USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	if (Context.Target.IsValid())
		return ScoreTagsOnActor(Context.Target.Get());

	return 0;
	
}
