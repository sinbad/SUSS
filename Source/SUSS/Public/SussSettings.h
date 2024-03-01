#pragma once

#include "CoreMinimal.h"
#include "SussSettings.generated.h"

/**
 * Settings for editor-specific aspects of SUDS (no effect at runtime)
 */
UCLASS(config = Suss, defaultconfig, meta=(DisplayName="SUSS"))
class SUSS_API USussSettings : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The frame time budget in milliseconds for running updates on AI brains"))
	float BrainUpdateFrameTimeBudgetMilliseconds = 0.5f;

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The interval at which a brain requests an update to its decision making, unless some other event forces them to request an update faster"))
	float BrainUpdateRequestIntervalSeconds = 1.0f;
	
};