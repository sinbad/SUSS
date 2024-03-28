// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussCommon.h"
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
	static bool IsActionEnabled(TSubclassOf<USussAction> ActionClass);
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

	static void AddEQSParams(const TMap<FName, FSussParameter>& Params, TArray<FEnvNamedValue>& OutQueryParams);

	static float EvalStepCurve(float Input, const FVector4f& Params);
	static float EvalLinearCurve(float Input, const FVector4f& Params);
	static float EvalQuadraticCurve(float Input, const FVector4f& Params);
	static float EvalExponentialCurve(float Input, const FVector4f& Params);
	static float EvalLogisticCurve(float Input, const FVector4f& Params);
	static float EvalCurve(ESussCurveType CurveType, float Input, const FVector4f& Params);

};
