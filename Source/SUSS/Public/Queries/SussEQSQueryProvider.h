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
	TArray<FEnvNamedValue> QueryConfig;

	/// The minimum score that the item from the EQS query must score to be included in the results. Usually you'd score
	/// your results in Considerations, but in case your EQS query does some scoring of its own and you want to use that
	/// as a threshold, include it here.
	UPROPERTY(EditDefaultsOnly, Category=Query)
	float MinScore = 0;

public:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	void OnPropertyChanged(const FName PropName);
#endif // WITH_EDITOR

protected:
	TSharedPtr<FEnvQueryResult> RunEQSQuery(USussBrainComponent* Brain,
	                                        AActor* Self,
	                                        const TMap<FName, FSussParameter>& Params,
	                                        const FSussContext& Context);
	bool ShouldIncludeResult(const FEnvQueryItem& Item) const;
};

/// Subclass this to provide a EQS-powered query which returns targets (actors)
UCLASS(Blueprintable)
class USussEQSTargetQueryProvider : public USussEQSQueryProvider
{
	GENERATED_BODY()
protected:
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, const FSussContext& BaseContext, TArray<TWeakObjectPtr<AActor>>& OutResults);

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<TWeakObjectPtr<AActor>>(OutResults);
		ExecuteQuery(Brain, Self, Params, FSussContext {Self}, GetResultsArray<TWeakObjectPtr<AActor>>(OutResults));
	}

	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override final
	{
		ExecuteQuery(Brain, Self, Params, Context, OutResults);
	}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<FVector>& OutResults) override final {}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<FSussContextValue>& OutResults) override final {}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Target; }
};

/// Subclass this to provide a EQS-powered query which returns locations
UCLASS(Blueprintable)
class USussEQSLocationQueryProvider : public USussEQSQueryProvider
{
	GENERATED_BODY()
protected:
	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, const FSussContext& BaseContext, TArray<FVector>& OutResults);

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<FVector>(OutResults);
		ExecuteQuery(Brain, Self, Params, FSussContext {Self}, GetResultsArray<FVector>(OutResults));
	}

	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<TWeakObjectPtr<AActor>>& OutResults) override final {}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<FVector>& OutResults) override final
	{
		ExecuteQuery(Brain, Self, Params, Context, OutResults);
	}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain,
		AActor* Self,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Params,
		TArray<FSussContextValue>& OutResults) override final {}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Location; }
};
