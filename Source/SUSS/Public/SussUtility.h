// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussCommon.h"
#include "SussParameter.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SussUtility.generated.h"

/**
 * 
 */
UCLASS()
class SUSS_API USussUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/// Global config can enable / disable just certain actions, so check this
	UFUNCTION(BlueprintCallable)
	static bool IsActionEnabled(FGameplayTag ActionTag);
	/// Check whether an actor has a specific tag
	UFUNCTION(BlueprintCallable)
	static bool ActorHasTag(AActor* Actor, const FGameplayTag& Tag);
	/// Check whether an actor has ANY of the supplied tags
	UFUNCTION(BlueprintCallable)
	static bool ActorHasAnyTags(AActor* Actor, const FGameplayTagContainer& Tags);
	/// Check whether an actor has ALL of the supplied tags
	UFUNCTION(BlueprintCallable)
	static bool ActorHasAllTags(AActor* Actor, const FGameplayTagContainer& Tags);

	static TSharedPtr<FEnvQueryResult> RunEQSQuery(UObject* WorldContextObject,
	                                               UEnvQuery* EQSQuery,
	                                               const TArray<FEnvNamedValue>& QueryParams,
	                                               EEnvQueryRunMode::Type QueryMode = EEnvQueryRunMode::AllMatching);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussFloatParameter(float Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussIntParameter(int Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussNameParameter(FName Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussTagParameter(FGameplayTag Val) { return FSussParameter(Val); }

	/**
	 * Manually run a query that returns locations, rather than use it to generate context for a brain decision.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Tag The tag identifying the query provider, should begin with "Suss.Query"
	 * @param Params Any parameters you wish to supply to the query
	 * @param UseCachedResultsFor If > 0, this query will return previous results for the same parameters rather than
	 *    running the query again, if it was already run within the last N seconds
	 * @return A list of locations from the query
	 */
	UFUNCTION(BlueprintCallable)
	static const TArray<FVector>& RunLocationQuery(AActor* Querier, FGameplayTag Tag, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor = 0);
	/**
	 * Manually run a query that returns target actors, rather than use it to generate context for a brain decision.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Tag The tag identifying the query provider, should begin with "Suss.Query"
	 * @param Params Any parameters you wish to supply to the query
	 * @param UseCachedResultsFor If > 0, this query will return previous results for the same parameters rather than
	 *    running the query again, if it was already run within the last N seconds
	 * @return A list of targets from the query
	 */
	UFUNCTION(BlueprintCallable)
	static TArray<AActor*> RunTargetQuery(AActor* Querier, FGameplayTag Tag, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor = 0);


	static void AddEQSParams(const TMap<FName, FSussParameter>& Params, TArray<FEnvNamedValue>& OutQueryParams);

	static float EvalStepCurve(float Input, const FVector4f& Params);
	static float EvalLinearCurve(float Input, const FVector4f& Params);
	static float EvalQuadraticCurve(float Input, const FVector4f& Params);
	static float EvalExponentialCurve(float Input, const FVector4f& Params);
	static float EvalLogisticCurve(float Input, const FVector4f& Params);
	static float EvalCurve(ESussCurveType CurveType, float Input, const FVector4f& Params);

};
