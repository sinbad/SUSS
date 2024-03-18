#include "SussQueryProvider.h"


uint32 USussQueryProvider::HashQueryRequest(AActor* Self, const TMap<FName, FSussParameter>& Params)
{
	uint32 Hash = 0;

	if (bSelfIsRelevant)
		Hash = GetTypeHash(Self);
	
	for (const auto& Pair : Params)
	{
		// Ignore parameters that are not used by this query provider
		if (!ParamNames.Contains(Pair.Key))
			continue;
		
		Hash = HashCombine(Hash, GetTypeHash(Pair.Key));
		Hash = HashCombine(Hash, GetTypeHash(Pair.Value));
	}
	return Hash;
}

bool USussQueryProvider::ParamsMatch(const TMap<FName, FSussParameter>& Params1,
                                     const TMap<FName, FSussParameter>& Params2) const
{
	// We *only* consider relevant params, to allow shared parameter lists with extra entries, without invalidating
	for (FName ParamName: ParamNames)
	{
		// missing on one side means invalidation
		if (Params1.Contains(ParamName) != Params2.Contains(ParamName))
		{
			return false;
		}

		// If both contain, check values match
		if (Params1.Contains(ParamName))
		{
			if (Params1[ParamName] != Params2[ParamName])
			{
				return false;
			}
		}
	}
	return true;
}
