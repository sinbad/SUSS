#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussInputProvider.h"
#include "SussParameter.h"
#include "SussBrainInfoInputProviders.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputTimeSinceActionPerformed);

/**
 * Input expressing the number of seconds since an action has been performed.
 * Required parameter "Tag" which is the tag for the action you're asking about.
 */
UCLASS()
class SUSS_API USussTimeSinceActionPerformedInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussTimeSinceActionPerformedInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

