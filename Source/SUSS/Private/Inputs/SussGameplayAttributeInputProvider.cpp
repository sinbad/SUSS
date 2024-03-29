// 
#include "Inputs/SussGameplayAttributeInputProvider.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

float USussGameplayAttributeInputProvider::GetAttributeValue(const AActor* FromActor) const
{
	if (IsValid(FromActor) && Attribute.IsValid())
	{
		if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(FromActor))
		{
			const float Val = ASC->GetNumericAttribute(Attribute);

			if (bNormaliseIfMaxAttributeSet && MaxAttribute.IsValid())
			{
				const float MaxVal = ASC->GetNumericAttribute(MaxAttribute);
				return FMath::GetRangePct(0.0f, MaxVal, Val);
			}
			else
			{
				return Val;
			}
		}
	}

	return 0;
}

float USussGameplayAttributeSelfInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
                                                                       const FSussContext& Context,
                                                                       const TMap<FName, FSussParameter>& Parameters) const
{
	return GetAttributeValue(Context.ControlledActor);
}

float USussGameplayAttributeTargetInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                         const FSussContext& Context,
                                                                         const TMap<FName, FSussParameter>& Parameters) const
{
	if (Context.Target.IsValid())
		return GetAttributeValue(Context.Target.Get());

	return 0;
}
