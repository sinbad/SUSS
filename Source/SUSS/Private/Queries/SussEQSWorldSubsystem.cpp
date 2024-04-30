// 
#include "Queries/SussEQSWorldSubsystem.h"

void USussEQSWorldSubsystem::SetTargetInfo(const AActor* Owner, AActor* Target)
{
	const uint32 Key = GetTargetKey(Owner);
	TargetContextMap.Add(Key, Target);
}

void USussEQSWorldSubsystem::ClearTargetInfo(const AActor* Owner)
{
	const uint32 Key = GetTargetKey(Owner);
	TargetContextMap.Remove(Key);
}

AActor* USussEQSWorldSubsystem::GetTargetInfo(AActor* Owner)
{
	const uint32 Key = GetTargetKey(Owner);
	if (auto pResult = TargetContextMap.Find(Key))
	{
		return *pResult;
	}
	return nullptr;
}
