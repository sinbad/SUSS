// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussContext.h"
#include "UObject/Object.h"
#include "SussQueryProvider.generated.h"


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

/**
 * Query providers are responsible for supplying some element of context for an input provider.
 * Input providers list all the queries they need running, and in turn the queries declare which elements
 * of the context they supply. The combination of all of these describes the full list of contexts that will be
 * generated for evaluation.
 *
 * Across all considerations for an action, only one query for each element of the context is allowed. Multiple
 * considerations can reference the same query (via their inputs), and that query will only be run once. Different
 * considerations  * can ask for different queries for different elements of the context, for example one consideration
 * could ask for targets to be queried, and another could ask for locations from a different query. But they could not
 * both ask for targets to be queried, unless the query being asked for is the same.
 *
 * Do NOT subclass from this base class. When setting up a query provider, you must:
 *   1. Subclass from one of the derived classes USussTargetQueryProvider, USussLocationQueryProvider etc
 *   2. Set QueryTag to some identifying value that begins with "Suss.Query". This is what others will refer to
 *   3. If your query needs parameters, add their names to ParamNames
 *   4. Register your provider with USussGameSubsystem, most easily via Project Settings > Plugins > SUSS > Query Providers
 */
UCLASS(Abstract)
class SUSS_API USussQueryProvider : public UObject
{
	GENERATED_BODY()
protected:

	/// The tag which identifies the query which this provider is supplying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag QueryTag;

	/// List of parameters which this query uses. MUST be correct to preserve cache info
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FName> ParamNames;

	/// The relevant parameters which were used to obtain cached results in the last execution
	/// May be a subset of the passed-in parameters, if we didn't use some of them.
	TMap<FName, float> CachedRelevantParams;

	/// The time period in seconds for which query results will be re-used rather than the query being re-executed
	UPROPERTY(EditDefaultsOnly)
	float ReuseResultsDuration = 0.5f;

	float TimeSinceLastRun = 100000;

public:

	USussQueryProvider() {}
	
	const FGameplayTag& GetQueryTag() const { return QueryTag; }
	const TArray<FName>& GetParamNames() const { return ParamNames; }

	// I'd prefer to make this pure virtual but UCLASS doesn't allow that
	virtual ESussQueryContextElement GetProvidedContextElement() const { return ESussQueryContextElement::Target; } 
	
	/// Immediately invalidate query results, meaning the next call will always perform the query again
	virtual void InvalidateResults() { TimeSinceLastRun = ReuseResultsDuration + 1000; }

	virtual void Tick(float DeltaTime);


protected:
	void MaybeExecuteQuery(const TMap<FName, float>& Params);
	/// Should be overridden by subclasses
	void ExecuteQuery(const TMap<FName, float>& Params) {}
	virtual bool ShouldUseCachedResults(const TMap<FName, float>& Params) const; 
	
};

/// Subclass this to provide a query which returns targets, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussTargetQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	TArray<TWeakObjectPtr<AActor>> CachedResults;
public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Target; }
	/// Retrieves the query results, using cached values if possible
	const TArray<TWeakObjectPtr<AActor>>& GetResults(const TMap<FName, float>& Params)
	{
		MaybeExecuteQuery(Params);
		return CachedResults;
	}
};

/// Subclass this to provide a query which returns locations, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussLocationQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	TArray<FVector> CachedResults;
public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Location; }
	/// Retrieves the query results, using cached values if possible
	const TArray<FVector>& GetResults(const TMap<FName, float>& Params)
	{
		MaybeExecuteQuery(Params);
		return CachedResults;
	}
};

/// Subclass this to provide a query which returns rotations, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussRotationQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	TArray<FRotator> CachedResults;
public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::Rotation; }
	/// Retrieves the query results, using cached values if possible
	const TArray<FRotator>& GetResults(const TMap<FName, float>& Params)
	{
		MaybeExecuteQuery(Params);
		return CachedResults;
	}
};

/// Subclass this to provide a query which returns custom context values, and register it with USussGameSubsystem
UCLASS(Abstract, Blueprintable)
class USussCustomValueQueryProvider : public USussQueryProvider
{
	GENERATED_BODY()
protected:
	TArray<TSussContextValue> CachedResults;
public:
	virtual ESussQueryContextElement GetProvidedContextElement() const override { return ESussQueryContextElement::CustomValue; }
	/// Retrieves the query results, using cached values if possible
	const TArray<TSussContextValue>& GetResults(const TMap<FName, float>& Params)
	{
		MaybeExecuteQuery(Params);
		return CachedResults;
	}
};