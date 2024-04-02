#pragma once

#include "CoreMinimal.h"
#include "SussInputProvider.h"
#include "SussQueryProvider.h"
#include "SussSettings.generated.h"

/**
 * Settings for editor-specific aspects of SUDS (no effect at runtime)
 */
UCLASS(config = Suss, defaultconfig, meta=(DisplayName="SUSS"))
class SUSS_API USussSettings : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of action classes to automatically register at startup"))
	TArray<TSubclassOf<USussAction>> ActionClasses;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of paths that Blueprint action classes will be searched for in", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> ActionClassPaths;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of input provider classes to automatically register at startup"))
	TArray<TSubclassOf<USussInputProvider>> InputProviders;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of paths that Blueprint input providers will be searched for in", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> InputProviderPaths;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of query providers classes to automatically register at startup"))
	TArray<TSubclassOf<USussQueryProvider>> QueryProviders;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of paths that Blueprint query providers will be searched for in", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> QueryProviderPaths;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "List of action tags which are globally disabled; useful for debugging, or disabling experimental behaviours"))
	FGameplayTagContainer DisabledActionTags;
	
	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The frame time budget in milliseconds for running updates on AI brains"))
	float BrainUpdateFrameTimeBudgetMilliseconds = 0.5f;

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The interval at which a brain requests an update to its decision making, unless some other event forces them to request an update faster"))
	float BrainUpdateRequestIntervalSeconds = 1.0f;

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "Whether perception changes trigger an immediate decision update of brains (e.g. spotting an enemy)"))
	bool BrainUpdateOnPerceptionChanges = true;
	
};