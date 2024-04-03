#include "SussQueryProvider.h"


uint32 USussQueryProvider::HashQueryRequest(AActor* Self, const TMap<FName, FSussParameter>& Params)
{
	uint32 Hash = 0;

	if (bSelfIsRelevant)
		Hash = GetTypeHash(Self);
	
	for (const auto& Pair : Params)
	{
		Hash = HashCombine(Hash, GetTypeHash(Pair.Key));
		Hash = HashCombine(Hash, GetTypeHash(Pair.Value));
	}
	return Hash;
}

bool USussQueryProvider::ParamsMatch(const TMap<FName, FSussParameter>& Params1,
                                     const TMap<FName, FSussParameter>& Params2) const
{
	if (Params1.Num() != Params2.Num())
		return false;
	
	for (auto ParamEntry: Params1)
	{
		if (auto pParam2 = Params2.Find(ParamEntry.Key))
		{
			if (ParamEntry.Value != *pParam2)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}
