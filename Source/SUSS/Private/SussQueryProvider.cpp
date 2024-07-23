#include "SussQueryProvider.h"

#include "SussCommon.h"


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

void USussTargetQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                            AActor* Self,
                                            const TMap<FName, FSussParameter>& Params,
                                            const FSussContext& Context,
                                            TArray<TWeakObjectPtr<AActor>>& OutResults)
{
	// Subclasses can override this, call BP version by default
	const TArray<AActor*> BPArray = ExecuteQueryBP(Brain, Self, Params, Context);
	OutResults.Append(BPArray);
}

void USussLocationQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                              AActor* Self,
                                              const TMap<FName, FSussParameter>& Params,
                                              const FSussContext& Context,
                                              TArray<FVector>& OutResults)
{
	// Subclasses can override this, call BP version by default
	const TArray<FVector> BPArray = ExecuteQueryBP(Brain, Self, Params, Context);
	OutResults.Append(BPArray);
}

void USussNamedValueQueryProvider::AddValueStruct(const TSharedPtr<const FSussContextValueStructBase>& Struct)
{
	TempOutArray->Add(FSussContextValue(Struct));
}

void USussNamedValueQueryProvider::AddValueStruct(const FSussContextValueStructBase* Struct)
{
	if (bUseCachedResults && !bDisableRawPointerCacheWarning)
	{
		UE_LOG(LogSuss, Warning, TEXT("%s uses raw pointers AND cacheing together, this is dangerous! Either set bUseCachedResults=false or bDisableRawPointerCacheWarning=true"), *StaticClass()->GetName());
	}
	TempOutArray->Add(FSussContextValue(Struct));
}

void USussNamedValueQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
	AActor* Self,
	const TMap<FName, FSussParameter>& Params,
	const FSussContext& Context,
	TArray<FSussContextValue>& OutResults)
{

	// Subclasses can override this, call BP version  by default
	// Array is not passed because it's BP-incompatible, use utility methods AddNamedValue.. 
	ExecuteQueryBP(Brain, Self, Params, Context);
}



