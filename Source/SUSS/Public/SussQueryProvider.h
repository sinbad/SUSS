// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SussContext.h"
#include "UObject/Object.h"
#include "SussParameter.h"
#include "SussQueryProvider.generated.h"


class USussBrainComponent;
/// Enum describing which element of the context a query is providing.
/// A query can only populate ONE element of a context; Target(s), OR Location(s), etc.
/// This enum indicates which the query is providing, because only one query can be run
/// for each element of the context within a given action.
UENUM(BlueprintType)
enum class ESussQueryContextElement : uint8
{
	/// Target actors
	Target,
	/// Locations in the world
	Location,
	/// Rotations in the world
	Rotation,
	/// Custom values
	CustomValue,
};

typedef TVariant<
		TArray<TWeakObjectPtr<AActor>>,
		TArray<FVector>,
		TArray<FRotator>,
		TArray<TSussContextValue>
	> TSussResultsArray;

struct FSussCachedQueryResults
{
public:
	TMap<FName, FSussParameter> Params;
	TWeakObjectPtr<AActor> ControlledActor;
	float TimeSinceLastRun = 100000;
	TSussResultsArray Results;
};
/**
 * Query providers are responsible for supplying building some element of context for action evaluation.
 * Action descriptions in a brain list all the queries they need running, and in turn the queries declare which elements
 * of the context they supply. The combination of all of these describes the full list of contexts that will be
 * generated for evaluation.
 *
 * Across all considerations for an action, only one query for each element of the context is allowed. Each action can
 * supply different parameters to the query.
 *
 * Do NOT subclass from this base class. When setting up a query provider, you must:
 *   1. Subclass from one of the derived classes USussTargetQueryProvider, USussLocationQueryProvider etc
 *   2. Set QueryTag to some identifying value that begins with "Suss.Query". This is what others will refer to
 *   3. If your query needs parameters, add their names to ParamNames (important!)
 *   4. Implement the ExecuteQuery function appropriate to your base class
 *   5. Register your provider with USussGameSubsystem, most easily via Project Settings > Plugins > SUSS > Query Providers
 */
UCLASS(Abstract)
class SUSS_API USussQueryProvider : public UPrimaryDataAsset
{
	GENERATED_BODY()
protected:

	/// The tag which identifies the query which this provider is supplying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag QueryTag;

	/// List of parameters which this query uses. MUST be correct to preserve cache info
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FName> ParamNames;

	/// Whether or not the value of "Self" (controlled actor) changes the results of this query
	/// You can set this to false as an optimisation if your query only accesses global information (or parameters),
	/// so that asking for results from any AI agent returns the same cached result instead of running the query again.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSelfIsRelevant = true;

	// Cached results for each params combination
	TMap<uint32, FSussCachedQueryResults> CachedResultsByParamsHash;

	template<typename T>
	static void InitResults(TSussResultsArray& OutResults)
	{
		if (!OutResults.IsType<TArray<T>>())
		{
			OutResults.Set<TArray<T>>(TArray<T>());
		}
		else
		{
			OutResults.Get<TArray<T>>().Reset();
		}
	}
	
	template<typename T>
	static TArray<T>& GetResultsArray(TSussResultsArray& Results)
	{
		check (Results.IsType<TArray<T>>())
		return Results.Get<TArray<T>>();
	}

	template<typename T>
	static const TArray<T>& GetResultsArray(const TSussResultsArray& Results)
	{
		check (Results.IsType<TArray<T>>())
		return Results.Get<TArray<T>>();
	}


public:

	USussQueryProvider() {}
	
	const FGameplayTag& GetQueryTag() const { return QueryTag; }
	const TArray<FName>& GetParamNames() const { return ParamNames; }

	bool GetSelfIsRelevant() const { return bSelfIsRelevant; }

	// I'd prefer to make this pure virtual but UCLASS doesn't allow that
	virtual ESussQueryContextElement GetProvidedContextElement() const { return ESussQueryContextElement::Target; } 

	virtual void Tick(float DeltaTime)
	{
		for (auto& Result : CachedResultsByParamsHash)
		{
			Result.Value.TimeSinceLastRun += DeltaTime;
		}
	}

	/// Retrieves the query results, using cached values if possible
	template<typename T>
	const TArray<T>& GetResults(USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params)
	{
		auto& Results = MaybeExecuteQuery(Brain, Self, MaxFrequency, Params, CachedResultsByParamsHash);
		return GetResultsArray<T>(Results.Results);
	}

protected:

	uint32 HashQueryRequest(AActor* Self, const TMap<FName, FSussParameter>& Params);
	bool ParamsMatch(const TMap<FName, FSussParameter>& Params1, const TMap<FName, FSussParameter>& Params2) const;

	bool ShouldUseCachedResults(const FSussCachedQueryResults& Results, USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params) const
	{
		// Always re-run if time has run out for cached results
		if (Results.TimeSinceLastRun >= MaxFrequency)
			return false;

		if (Results.ControlledActor.Get() != Self)
			return false;

		// Otherwise, if we're within the re-use time, the only time we should not re-use is if the value of a relevant
		// parameter is different. Relevant parameters for queries may be a subset of the total parameter list
		return ParamsMatch(Results.Params, Params);
	}

	virtual void ExecuteQueryInteral(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults)
	{
		// Subclass specific
	}
	
	void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, FSussCachedQueryResults& OutResults)
	{
		OutResults.Params = Params;
		OutResults.ControlledActor = Self;
		OutResults.TimeSinceLastRun = 0;
		ExecuteQueryInteral(Brain, Self, Params, OutResults.Results);
	}

	const FSussCachedQueryResults& MaybeExecuteQuery(USussBrainComponent* Brain,
	                                                 AActor* Self,
	                                                 float MaxFrequency,
	                                                 const TMap<FName, FSussParameter>& Params,
	                                                 TMap<uint32, FSussCachedQueryResults>& CachedResults)
	{
		const uint32 ParamsHash = HashQueryRequest(Self, Params);
		if (const auto pResultStruct = CachedResults.Find(ParamsHash))
		{
			if (!ShouldUseCachedResults(*pResultStruct, Brain, Self, MaxFrequency, Params))
			{
				// Run query again, but re-use cache entry. Reset to keep allocations
				ExecuteQuery(Brain, Self, Params, *pResultStruct);
			}
			return *pResultStruct;
		}

		// First run of this query
		auto& ResultStruct = CachedResults.Emplace(ParamsHash);
		ExecuteQuery(Brain, Self, Params, ResultStruct);
		return ResultStruct;
	}
	
};

/// Subclass this to provide a query which returns targets, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussTargetQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	
	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<TWeakObjectPtr<AActor>>& OutResults) {}

	virtual void ExecuteQueryInteral(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<TWeakObjectPtr<AActor>>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<TWeakObjectPtr<AActor>>(OutResults));
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Target; }

};

/// Subclass this to provide a query which returns locations, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussLocationQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:

	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<FVector>& OutResults) {}

	virtual void ExecuteQueryInteral(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<FVector>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<FVector>(OutResults));
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Location; }
};

/// Subclass this to provide a query which returns rotations, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussRotationQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<FRotator>& OutResults) {}

	virtual void ExecuteQueryInteral(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<FRotator>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<FRotator>(OutResults));
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Rotation; }

};

/// Subclass this to provide a query which returns custom context values, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussCustomValueQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:

	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<TSussContextValue>& OutResults) {}

	virtual void ExecuteQueryInteral(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<TSussContextValue>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<TSussContextValue>(OutResults));
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::CustomValue; }

};