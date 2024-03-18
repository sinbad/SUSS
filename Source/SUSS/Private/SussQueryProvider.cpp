#include "SussQueryProvider.h"

void USussQueryProvider::Tick(float DeltaTime)
{
	TimeSinceLastRun += DeltaTime;
}

void USussQueryProvider::MaybeExecuteQuery(USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params)
{
	if (!ShouldUseCachedResults(Brain, Self, MaxFrequency, Params))
	{
		// Store the subset of params we need
		CachedRelevantParams = Params.FilterByPredicate([this](const TMap<FName, FSussParameter>::ElementType& Elem)
		{
			return ParamNames.Contains(Elem.Key);
		});
		CachedSelf = Self;
		ExecuteQuery(Brain, Self, CachedRelevantParams);
	}
}

bool USussQueryProvider::ShouldUseCachedResults(USussBrainComponent* Brain, AActor* Self, float MaxFrequency, const TMap<FName, FSussParameter>& Params) const
{
	// Always re-run if time has run out for cached results
	if (TimeSinceLastRun >= MaxFrequency)
		return false;

	if (CachedSelf.Get() != Self)
		return false;

	// Otherwise, if we're within the re-use time, the only time we should not re-use is if the value of a relevant
	// parameter is different. Relevant parameters for queries may be a subset of the total parameter list
	for (FName ParamName: ParamNames)
	{
		// missing on one side means invalidation
		if (CachedRelevantParams.Contains(ParamName) != Params.Contains(ParamName))
		{
			return false;
		}

		// If both contain, check values match
		if (CachedRelevantParams.Contains(ParamName))
		{
			if (CachedRelevantParams[ParamName] != Params[ParamName])
			{
				return false;
			}
		}
	}

	// All good, re-use
	return true;
}

