#include "Inputs/SussBrainInfoInputProviders.h"

#include "SussBrainComponent.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTimeSinceActionPerformed, "Suss.Input.Brain.Actions.TimeSincePerformed", "Get the number of seconds since an action was last performed. Requires parameter 'Tag' which is the action tag you're querying.")

USussTimeSinceActionPerformedInputProvider::USussTimeSinceActionPerformedInputProvider()
{
	InputTag = TAG_SussInputTimeSinceActionPerformed;
}

float USussTimeSinceActionPerformedInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	if (auto pParam = Parameters.Find(SUSS::TagParamName))
	{
		return Brain->GetTimeSinceActionPerformed(pParam->Tag);
	}

	return UE_BIG_NUMBER;
}
