// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussCommon.h"
#include "SussParameter.h"
#include "SussContext.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SussUtility.generated.h"

struct FSussActorPerceptionInfo;
class AAIController;
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
	UFUNCTION(BlueprintCallable, DisplayName="Run EQS Query (SUSS)", meta=(WorldContext=WorldContextObject))
	static UEnvQueryInstanceBlueprintWrapper* RunEQSQueryBP(AActor* Querier,
	                                                        UEnvQuery* EQSQuery,
	                                                        const TArray<FEnvNamedValue>& QueryParams,
	                                                        EEnvQueryRunMode::Type QueryMode =
		                                                        EEnvQueryRunMode::AllMatching);

	static TSharedPtr<FEnvQueryResult> RunEQSQueryWithTargetContext(AActor* Querier,
	                                                                AActor* Target,
	                                                                UEnvQuery* EQSQuery,
	                                                                const TMap<FName, FSussParameter>& Params,
	                                                                TEnumAsByte<EEnvQueryRunMode::Type> QueryMode =
		                                                                EEnvQueryRunMode::AllMatching);
	/**
	 * Manually run an EQS query with a target context.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Target The actor which is to be the "Target" as referenced in the EQS context
	 * @param EQSQuery The EQS query
	 * @param Params Any extra parameters you wish to supply to the query
	 * @param QueryMode What to return
	 * @return EQS query results
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Run EQS Query With Target Context (SUSS)", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "WrapperClass"))
	static UEnvQueryInstanceBlueprintWrapper* RunEQSQueryWithTargetContextBP(AActor* Querier,
	                                                                         AActor* Target,
	                                                                         UEnvQuery* EQSQuery,
	                                                                         const TMap<FName, FSussParameter>& Params,
	                                                                         TEnumAsByte<EEnvQueryRunMode::Type> QueryMode =
		                                                                         EEnvQueryRunMode::AllMatching);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussFloatParameter(float Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussIntParameter(int Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussNameParameter(FName Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussTagParameter(FGameplayTag Val) { return FSussParameter(Val); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSussParameter MakeSussTagContainerParameter(const FGameplayTagContainer& Val) { return FSussParameter(Val); }
	/**
	 * Try to extract a float value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The float value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsFloat(const FSussParameter& Parameter, float& Value);
	/**
	 * Try to extract a vector value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The vector value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsVector(const FSussParameter& Parameter, FVector& Value);	
	/**
	 * Try to extract a boolean value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The boolean value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsBool(const FSussParameter& Parameter, bool& Value);	
	/**
	 * Try to extract an integer value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The integer value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsInt(const FSussParameter& Parameter, int& Value);
	/**
	 * Try to extract a name value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The name value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsName(const FSussParameter& Parameter, FName& Value);
	/**
	 * Try to extract a tag value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The tag value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsTag(const FSussParameter& Parameter, FGameplayTag& Value);
	/**
	 * Try to extract a tag container value from a parameter.
	 * @param Parameter The parameter, which may contain many types of value
	 * @param Value The tag container value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussParameterValueAsTagContainer(const FSussParameter& Parameter, FGameplayTagContainer& Value);
	/**
	 * Try to extract a named Actor value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The Actor value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsActor(const FSussContext& Context, FName Name, AActor*& Value);
	/**
	 * Try to extract a named float value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The float value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsFloat(const FSussContext& Context, FName Name, float& Value);
	/**
	 * Try to extract a named vector value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The vector value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsVector(const FSussContext& Context, FName Name, FVector& Value);	
	/**
	 * Try to extract a named rotator value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The rotator value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsRotator(const FSussContext& Context, FName Name, FRotator& Value);	

	/**
	 * Try to extract an named integer value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The integer value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsInt(const FSussContext& Context, FName Name, int& Value);
	/**
	 * Try to extract a named name value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The name value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsName(const FSussContext& Context, FName Name, FName& Value);
	/**
	 * Try to extract a named tag value from a context.
	 * @param Context The context
	 * @param Name The name of the value
	 * @param Value The name value
	 * @return True if the value was extracted correctly. False if not.
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static UPARAM(DisplayName="Success") bool GetSussContextValueAsTag(const FSussContext& Context, FName Name, FGameplayTag& Value);

	/**
	 * Manually run a query that returns locations, rather than use it to generate context for a brain decision.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Tag The tag identifying the query provider, should begin with "Suss.Query"
	 * @param Params Any contexts you wish to supply to the query
	 * @param UseCachedResultsFor If > 0, this query will return previous results for the same contexts rather than
	 *    running the query again, if it was already run within the last N seconds
	 * @return A list of locations from the query
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static const TArray<FVector>& RunLocationQuery(AActor* Querier, FGameplayTag Tag, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor = 0);

	/**
	 * Manually run a query that returns locations, rather than use it to generate context for a brain decision, and supply a target context.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Tag The tag identifying the query provider, should begin with "Suss.Query"
	 * @param Target The actor which is to be the "Target" as referenced in the EQS context
	 * @param Params Any contexts you wish to supply to the query
	 * @param UseCachedResultsFor If > 0, this query will return previous results for the same contexts rather than
	 *    running the query again, if it was already run within the last N seconds
	 * @return A list of locations from the query
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static const TArray<FVector>& RunLocationQueryWithTargetContext(AActor* Querier, FGameplayTag Tag, AActor* Target, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor = 0);
	/**
	 * Manually run a query that returns target actors, rather than use it to generate context for a brain decision.
	 * You might want to do this if you want some query results to manually choose inside an action, rather than evaluating
	 * all the results individually as contexts.
	 * @param Querier The actor which is the "Querier", usually the controlled actor, or self.
	 * @param Tag The tag identifying the query provider, should begin with "Suss.Query"
	 * @param Params Any contexts you wish to supply to the query
	 * @param UseCachedResultsFor If > 0, this query will return previous results for the same contexts rather than
	 *    running the query again, if it was already run within the last N seconds
	 * @return A list of targets from the query
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static TArray<AActor*> RunTargetQuery(AActor* Querier, FGameplayTag Tag, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor = 0);
	
	static void AddEQSParams(const TMap<FName, FSussParameter>& Params, TArray<FEnvNamedValue>& OutQueryParams);

	static float EvalStepCurve(float Input, const FVector4f& Params);
	static float EvalLinearCurve(float Input, const FVector4f& Params);
	static float EvalQuadraticCurve(float Input, const FVector4f& Params);
	static float EvalExponentialCurve(float Input, const FVector4f& Params);
	static float EvalLogisticCurve(float Input, const FVector4f& Params);
	static float EvalCurve(ESussCurveType CurveType, float Input, const FVector4f& Params);

	/**
	 * Get the distance along navmesh paths from an actor's current location to a desired location.
	 * @param Agent The actor in question
	 * @param Location The desired location 
	 * @return Distance, or BIG_NUMBER if unreachable
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static float GetPathDistanceTo(AAIController* Agent, const FVector& Location);

	/**
	 * Get the distance along navmesh paths between 2 locations, for an actor.
	 * @param Agent The actor in question
	 * @param FromLocation The location to measure from
	 * @param ToLocation The desired location 
	 * @return Distance, or BIG_NUMBER if unreachable
	 */
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static float GetPathDistanceFromTo(AAIController* Agent, const FVector& FromLocation, const FVector& ToLocation);

	UFUNCTION(Blueprintable, Category="SUSS")
	static ECollisionChannel GetLineOfSightTraceChannel();

	/// Helper function to retrieve the "PerceptionInfo" named struct from a context
	UFUNCTION(BlueprintCallable, Category="SUSS")
	static const FSussActorPerceptionInfo& GetPerceptionInfoFromContext(const FSussContext& Context, bool& bSuccess);
};
