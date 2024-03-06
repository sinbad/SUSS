#include "SussQueryProvider.h"

void USussQueryProvider::Tick(float DeltaTime)
{
	TimeSinceLastRun += DeltaTime;
}

void USussQueryProvider::MaybeExecuteQuery(const TMap<FName, float>& Params)
{
	if (!ShouldUseCachedResults(Params))
	{
		// Store the subset of params we need
		CachedRelevantParams = Params.FilterByPredicate([this](const TMap<FName, float>::ElementType& Elem)
		{
			return ParamNames.Contains(Elem.Key);
		});
		ExecuteQuery(CachedRelevantParams);
	}
}

bool USussQueryProvider::ShouldUseCachedResults(const TMap<FName, float>& Params) const
{
	// Always re-run if time has run out for cached results
	if (TimeSinceLastRun > ReuseResultsDuration)
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

