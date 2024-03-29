// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussInputProvider.h"
#include "SussPerceptionInputProviders.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputSelfSightRange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputSelfHearingRange);
/**
 * Input providing the sight range of the controlled actor (self)
 */
UCLASS()
class SUSS_API USussSelfSightRangeInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussSelfSightRangeInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input providing the hearing range of the controlled actor (self)
 */
UCLASS()
class SUSS_API USussSelfHearingRangeInputProvider : public USussInputProvider
{
	GENERATED_BODY()
public:
	USussSelfHearingRangeInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};
