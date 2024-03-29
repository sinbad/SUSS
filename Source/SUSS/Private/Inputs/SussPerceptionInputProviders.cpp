
#include "..\..\Public\Inputs\SussPerceptionInputProviders.h"

#include "SussBrainComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputSelfSightRange, "Suss.Input.Perception.Sight.RangeSelf", "Get the sight range of the controlled actor")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputSelfHearingRange, "Suss.Input.Perception.Hearing.RangeSelf", "Get the hearing range of the controlled actor")

USussSelfSightRangeInputProvider::USussSelfSightRangeInputProvider()
{
	InputTag = TAG_SussInputSelfSightRange;
}

float USussSelfSightRangeInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                const FSussContext& Ctx,
                                                                const TMap<FName, FSussParameter>& Parameters) const
{
	if (const auto Percept = Brain->GetPerceptionComponent())
	{
		if (const auto SenseCfg = Cast<UAISenseConfig_Sight>(Percept->GetSenseConfig(GetDefault<UAISense_Sight>()->GetSenseID())))
		{
			// Use the outer lose sight radius
			return SenseCfg->LoseSightRadius;
		}
	}

	return 1000;
}

USussSelfHearingRangeInputProvider::USussSelfHearingRangeInputProvider()
{
	InputTag = TAG_SussInputSelfHearingRange;
}

float USussSelfHearingRangeInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                  const FSussContext& Ctx,
                                                                  const TMap<FName, FSussParameter>& Parameters) const
{
	if (const auto Percept = Brain->GetPerceptionComponent())
	{
		if (const auto SenseCfg = Cast<UAISenseConfig_Hearing>(Percept->GetSenseConfig(GetDefault<UAISense_Hearing>()->GetSenseID())))
		{
			// Use the outer lose sight radius
			return SenseCfg->HearingRange;
		}
	}

	return 100;
}
