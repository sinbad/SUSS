// 

#pragma once

#include "CoreMinimal.h"
#include "SussQueryProvider.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "SussEQSQueryProvider.generated.h"

/**
 * This query provider delegates to an EQS query. Use one of the subclasses for specific result types (e.g. Location)
 */
UCLASS(Abstract)
class SUSS_API USussEQSQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	/// Link to the EQS query which this provider will run
	UPROPERTY(EditDefaultsOnly, Category=Query)
	UEnvQuery* EQSQuery;

	/// How many results we're looking for from this query
	UPROPERTY(EditDefaultsOnly, Category=Query)
	TEnumAsByte<EEnvQueryRunMode::Type> QueryMode = EEnvQueryRunMode::AllMatching;

	/// EQS parameter values which are static. Note that instance params can still be supplied when this query is used
	UPROPERTY(EditDefaultsOnly, Category=Query)
	TArray<FAIDynamicParam> QueryConfig;


public:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	void OnPropertyChanged(const FName PropName);
#endif // WITH_EDITOR

protected:
	bool CheckResultClass(TSubclassOf<UEnvQueryItemType> ResultClass) { return false; } // needs to be overridden
	void RunEQSQuery(USussBrainComponent* Brain,
	                 AActor* Self,
	                 const TMap<FName, FSussParameter>& Params,
	                 TFunctionRef<void(const FEnvQueryItem& Result)> ResultCallback);
};
