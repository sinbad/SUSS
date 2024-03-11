// 

#pragma once

#include "CoreMinimal.h"
#include "SussBrainComponent.h"
#include "Engine/DataAsset.h"
#include "SussBrainConfigAsset.generated.h"

/**
 * A brain config asset is a separately defineable brain configuration which can then be referenced to set / change the
 * behaviour of your AIs without having to define a new AIController for every type of enemy. You can just define brain
 * config assets (and optionally, ActionSet assets if you want to re-use sets of actions) and then set those as you
 * initialise your AIs, with a common AIController.
 */
UCLASS()
class SUSS_API USussBrainConfigAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FSussBrainConfig BrainConfig;
};
