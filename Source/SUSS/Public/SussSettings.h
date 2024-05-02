#pragma once

#include "CoreMinimal.h"
#include "SussInputProvider.h"
#include "SussParameterProvider.h"
#include "SussQueryProvider.h"
#include "SussSettings.generated.h"


USTRUCT(BlueprintType)
struct FSussAgentDistanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ToolTip = "The maximum distance of an agent from any player at which these update settings apply"))
	float MaxDistance = 1000;

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The interval at which a brain requests an update to its decision making at this distance, unless some other event forces them to request an update faster"))
	float BrainUpdateRequestIntervalSeconds = 1.0f;

};
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

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of parameter provider classes to automatically register at startup"))
	TArray<TSubclassOf<USussParameterProvider>> ParameterProviders;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (ToolTip = "A list of paths that Blueprint parameter providers will be searched for in", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> ParameterProviderPaths;

	UPROPERTY(config, EditAnywhere, Category = BaseConfiguration, meta = (Categories="Suss.Action", ToolTip = "List of action tags which are globally disabled; useful for debugging, or disabling experimental behaviours"))
	FGameplayTagContainer DisabledActionTags;
	
	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The frame time budget in milliseconds for running updates on AI brains"))
	float BrainUpdateFrameTimeBudgetMilliseconds = 0.5f;
	
	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "Whether perception changes trigger an immediate decision update of brains (e.g. spotting an enemy)"))
	bool BrainUpdateOnPerceptionChanges = true;

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "Settings related for agents near to any player"))
	FSussAgentDistanceSettings NearAgentSettings = {1000, 0.1f };

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "Settings related for agents at middling range to any player"))
	FSussAgentDistanceSettings MidRangeAgentSettings = {5000, 1.0f };

	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "Settings related for agents far from any player. Any agents more distant than this will not be updated."))
	FSussAgentDistanceSettings FarAgentSettings = {10000, 3.0f };
	
	UPROPERTY(config, EditAnywhere, Category = Optimisation, meta = (ToolTip = "The interval at which we'll re-calculate the distance to the players when the agent is beyond the far distance"))
	float OutOfBoundsDistanceCheckInterval = 3;

	UPROPERTY(config, EditAnywhere, Category = Collision, meta = (ToolTip = "The trace channel to use when determining Line of Sight tests. Defaults to Visibility but if you want AI to avoid shooting each other you might want to use a custom trace."))
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_Visibility;
};