#include "SussQueryProvider.h"

void USussQueryProvider::Tick(float DeltaTime)
{
	TimeSinceLastRun += DeltaTime;
}

void USussQueryProvider::MaybeExecuteQuery(const TMap<FName, FSussParameter>& Params)
{
	if (!ShouldUseCachedResults(Params))
	{
		ExecuteQuery(Params);
	}
}

bool USussQueryProvider::ShouldUseCachedResults(const TMap<FName, FSussParameter>& Params) const
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

