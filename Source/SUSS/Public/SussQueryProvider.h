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
	/// Named values of various types
	NamedValue,
};

typedef TVariant<
		TArray<TWeakObjectPtr<AActor>>,
		TArray<FVector>,
		TArray<FSussContextValue>
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
 * Query providers are responsible for supplying some element of context for action evaluation, e.g. a location, or a target.
 * Action descriptions in a brain list all the queries they need running, and in turn the queries declare which elements
 * of the context they supply. The combination of all of these describes the full list of contexts that will be
 * generated for evaluation.
 *
 * Across all considerations for an action, only one query for each element of the context is allowed. Each action can
 * supply different parameters to the query, allowing you to reuse more general purpose queries.
 *
 * The results of multiple queries on a single action can be combined in two ways - "correlated" or "uncorrelated".
 * 
 * "Uncorrelated" means that each query is run *once* and returns its own set of results irrespective of
 * other queries. Each set of results is combined with other queries by generating every possible combination, e.g.
 * if Query1 returned 3 targets, and Query2 returned 2 locations, this would generate 6 (3x2) contexts in total
 * with every combination of those results.
 *
 * "Correlated" queries generate results based on the results of previous queries. They are run *once per previous result*
 * and receive a context from the previous queries for each invocation. They then generate one or more values related to
 * that context which are then combined. For example if Query1 returned 3 targets, Query2 would be run 3 times, and
 * the location results for each run would be combined with just the one target in that invocation (and not the others). 
 * Location 1 might make Query2 generate 2 locations, but locations 1 and 2 might generate none; in which case there would
 * only be 2 contexts from both queries.
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Suss.Query"))
	FGameplayTag QueryTag;

	/// Whether this query is "correlated" with the results of previous queries. If true the query is called once for
	/// every context generated from previous queries in the action, allowing these results to be generated contextually
	/// with the results from other queries: e.g. "generate locations around a target", where the list of targets was
	/// generated from another query.
	/// If false this query will simply generate values independently of any other query. The first query in an action
	/// has to be uncorrelated.
	/// Note: Correlated queries can never cache their results.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsCorrelatedWithContext = false;

	/// Whether or not the value of "Self" (controlled actor) changes the results of this query
	/// You can set this to false as an optimisation if your query only accesses global information (or parameters),
	/// so that asking for results from any AI agent returns the same cached result instead of running the query again.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSelfIsRelevant = true;

	/// Whether or not this query should re-use cached results within the max requested frequency
	/// You might want to set this to false if your query just reads already prepared data from elsewhere, which is
	/// updated only when needed, and thus when the query fires you always want the latest from that. E.g. perception.
	/// Note: Correlated queries can never cache their results.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseCachedResults = true;

	/// Set this to true if you're using raw pointers to structs as results (C++ only) and want to keep caching results
	/// without having warnings all the time. Use with caution! You must be absolutely sure that the structs the cached
	/// results point to will outlive the cache.
	bool bDisableRawPointerCacheWarning = false; 

	// Cached results for each params combination
	TMap<uint32, FSussCachedQueryResults> CachedResultsByParamsHash;

	mutable FCriticalSection Guard;

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
	
	virtual FGameplayTag GetQueryTag() const { return QueryTag; }

	bool GetSelfIsRelevant() const { return bSelfIsRelevant; }

	// I'd prefer to make this pure virtual but UCLASS doesn't allow that
	virtual ESussQueryContextElement GetProvidedContextElement() const { return ESussQueryContextElement::Target; } 

	virtual void Tick(float DeltaTime)
	{
		FScopeLock Lock(&Guard);
		
		TArray<uint32> KeysToRemove;
		for (auto& Result : CachedResultsByParamsHash)
		{
			// If the AI that used to use this has gone stale, remove
			if (Result.Value.ControlledActor.IsStale() || !Result.Value.ControlledActor.IsValid())
			{
				KeysToRemove.Add(Result.Key);
			}
			else
			{
				Result.Value.TimeSinceLastRun += DeltaTime;
			}
		}

		for (uint32 Key : KeysToRemove)
		{
			CachedResultsByParamsHash.Remove(Key);
		}
	}

	/// Retrieves the query results, using cached values if possible
	template<typename T>
	const TArray<T>& GetResults(USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params)
	{
		FScopeLock Lock(&Guard);
		
		auto& Results = MaybeExecuteQuery(Brain, Self, MaxFrequency, Params, CachedResultsByParamsHash);
		return GetResultsArray<T>(Results.Results);
	}

	/// Run the query, correlated with an existing context generated from another query
	/// Note: results are never cached on correlated queries.
	template<typename T>
	void GetResultsInContext(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<T>& OutResults)
	{
		// No caching, direct call through
		ExecuteQueryInContext(Brain, Self, Context, Params, OutResults);
	}

protected:

	uint32 HashQueryRequest(AActor* Self, const TMap<FName, FSussParameter>& Params);
	bool ParamsMatch(const TMap<FName, FSussParameter>& Params1, const TMap<FName, FSussParameter>& Params2) const;

	virtual bool ShouldUseCachedResults(const FSussCachedQueryResults& Results, USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params) const
	{
		// If we're not caching, we don't use (we still use cache as a store since we return by ref)
		if (!bUseCachedResults)
			return false;
		
		// Always re-run if time has run out for cached results
		if (Results.TimeSinceLastRun >= MaxFrequency)
			return false;

		if (Results.ControlledActor.Get() != Self)
			return false;

		// Otherwise, if we're within the re-use time, the only time we should not re-use is if the value of a relevant
		// parameter is different. Relevant parameters for queries may be a subset of the total parameter list
		return ParamsMatch(Results.Params, Params);
	}

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults)
	{
		// Subclass specific
	}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<TWeakObjectPtr<AActor>>& OutResults)
	{
		// Subclass specific
	}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FVector>& OutResults)
	{
		// Subclass specific
	}
	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FSussContextValue>& OutResults)
	{
		// Subclass specific
	}
	
	void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, FSussCachedQueryResults& OutResults)
	{
		OutResults.Params = Params;
		OutResults.ControlledActor = Self;
		OutResults.TimeSinceLastRun = 0;
		ExecuteQueryInternal(Brain, Self, Params, OutResults.Results);
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

/// Subclass this to provide a query which returns targets
UCLASS(Abstract, Blueprintable)
class SUSS_API USussTargetQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	
	/// Should be overridden by subclasses for uncorrelated queries
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<TWeakObjectPtr<AActor>>& OutResults);
	/// Should be overridden by subclasses for correlated queries
	virtual void ExecuteQueryInContext(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<TWeakObjectPtr<AActor>>& OutResults);

	/// Implement query for uncorrelated queries
	// BPs cannot use weak object pointers so this has to proxy
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQuery")
	void ExecuteQueryBP(USussBrainComponent* Brain, AActor* ControlledActor, const TMap<FName, FSussParameter>& Params, UPARAM(ref) TArray<AActor*>& OutResults);
	/// Implement query for correlated queries (based on previous contexts)
	// BPs cannot use weak object pointers so this has to proxy
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQueryInContext")
	void ExecuteQueryInContextBP(USussBrainComponent* Brain, AActor* ControlledActor, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, UPARAM(ref) TArray<AActor*>& OutResults);

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<TWeakObjectPtr<AActor>>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<TWeakObjectPtr<AActor>>(OutResults));
	}

	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<TWeakObjectPtr<AActor>>& OutResults) override final
	{
		ExecuteQueryInContext(Brain, Self, Context, Params, OutResults);
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Target; }

};

/// Subclass this to provide a query which returns locations
UCLASS(Abstract, Blueprintable)
class SUSS_API USussLocationQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:

	/// Should be overridden by subclasses for uncorrelated queries
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<FVector>& OutResults);
	/// Should be overridden by subclasses for correlated queries
	virtual void ExecuteQueryInContext(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FVector>& OutResults);

	/// Implement for uncorrelated queries
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQuery")
	void ExecuteQueryBP(USussBrainComponent* Brain, AActor* ControlledActor, const TMap<FName, FSussParameter>& Params, UPARAM(ref) TArray<FVector>& OutResults);
	/// Implement for correlated queries
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQueryInContext")
	void ExecuteQueryInContextBP(USussBrainComponent* Brain, AActor* ControlledActor, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, UPARAM(ref) TArray<FVector>& OutResults);

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<FVector>(OutResults);
		ExecuteQuery(Brain, Self, Params, GetResultsArray<FVector>(OutResults));
	}

	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FVector>& OutResults) override final
	{
		ExecuteQueryInContext(Brain, Self, Context, Params, OutResults);
	}

public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Location; }
};

/// Subclass this to provide a query which returns named context values
/// Each query must only return items of a single type, for a single name
UCLASS(Abstract, Blueprintable)
class SUSS_API USussNamedValueQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:

	/// The name of the value that this query will be returning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName QueryValueName;

	/// The type of the value that this query will be returning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ESussContextValueType QueryValueType = ESussContextValueType::Float;

	// We need to hold this to allow BP to fill in
	TArray<FSussContextValue>* TempOutArray;

	UFUNCTION(BlueprintCallable)
	void AddValueActor(AActor* Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueVector(FVector Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueRotator(FRotator Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueTag(FGameplayTag Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueName(FName Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueFloat(float Value) { TempOutArray->Add(FSussContextValue(Value)); }
	UFUNCTION(BlueprintCallable)
	void AddValueInt(int Value) { TempOutArray->Add(FSussContextValue(Value)); }
	
	/// Add a struct as a shared pointer (C++ only)
	void AddValueStruct(const TSharedPtr<const FSussContextValueStructBase>& Struct);
	/// Add a struct as a raw pointer (C++ only)
	/// STRONGLY advise to set bUseCachedResults=false unless you *know* for sure that what's pointed to will outlive the cache
	void AddValueStruct(const FSussContextValueStructBase* Struct);


	/// Should be overridden by subclasses
	virtual void ExecuteQuery(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TArray<FSussContextValue>& OutResults);
	/// Should be overridden by subclasses
	virtual void ExecuteQueryInContext(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FSussContextValue>& OutResults);

	// Implement in BP to execute uncorrelated query
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQuery")
	void ExecuteQueryBP(USussBrainComponent* Brain, AActor* ControlledActor, const TMap<FName, FSussParameter>& Params);
	// Implement in BP to execute correlated query
	UFUNCTION(BlueprintImplementableEvent, DisplayName="ExecuteQueryInContext")
	void ExecuteQueryInContextBP(USussBrainComponent* Brain, AActor* ControlledActor, const FSussContext& Context, const TMap<FName, FSussParameter>& Params);

	virtual void ExecuteQueryInternal(USussBrainComponent* Brain, AActor* Self, const TMap<FName, FSussParameter>& Params, TSussResultsArray& OutResults) override final
	{
		InitResults<FSussContextValue>(OutResults);
		// We have to store local version so BP can interact using helper functions
		TempOutArray = &(OutResults.Get<TArray<FSussContextValue>>());
		ExecuteQuery(Brain, Self, Params, GetResultsArray<FSussContextValue>(OutResults));
		TempOutArray = nullptr;

	}

	virtual void ExecuteQueryInContextInternal(USussBrainComponent* Brain, AActor* Self, const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TArray<FSussContextValue>& OutResults) override final
	{
		// We have to store local version so BP can interact using helper functions
		TempOutArray = &OutResults;
		ExecuteQueryInContext(Brain, Self, Context, Params, OutResults);
		TempOutArray = nullptr;

	}

public:
	const FName& GetQueryValueName() const
	{
		return QueryValueName;
	}

	ESussContextValueType GetQueryValueType() const
	{
		return QueryValueType;
	}

	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::NamedValue; }

};