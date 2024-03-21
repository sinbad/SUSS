// 


#include "SussPoolSubsystem.h"

#include "SussAction.h"
#include "SussCommon.h"

FSussScopeReservedArray::~FSussScopeReservedArray()
{
	if (OwningSystem.IsValid())
	{
		OwningSystem->FreeArray(Holder);
	}
	else
	{
		// This should never happen
		UE_LOG(LogSuss, Error, TEXT("FSussScopeReservedArray destroyed after owning subsystem, this is a memory leak!"))
	}
}

FSussScopeReservedMap::~FSussScopeReservedMap()
{
	if (OwningSystem.IsValid())
	{
		OwningSystem->FreeMap(Holder);
	}
	else
	{
		// This should never happen
		UE_LOG(LogSuss, Error, TEXT("FSussScopeReservedMap destroyed after owning subsystem, this is a memory leak!"))
	}
}

USussAction* USussPoolSubsystem::ReserveAction(const UClass* ActionClass,
	UObject* OwnerIfCreated,
	UObject* TemplateIfCreated)
{
	FScopeLock Lock(&Guard);
	
	if (auto FreeList = FreeActionClassPools.Find(ActionClass))
	{
		if (FreeList->Pool.Num() > 0)
		{
			return FreeList->Pool.Pop();
		}
	}

	return 	NewObject<USussAction>(OwnerIfCreated, ActionClass, NAME_None, RF_NoFlags, TemplateIfCreated);

}

void USussPoolSubsystem::FreeAction(USussAction* Action)
{
	FScopeLock Lock(&Guard);
	
	auto FreeList = FreeActionClassPools.Find(Action->GetClass());
	if (!FreeList)
	{
		FreeList = &FreeActionClassPools.Emplace(Action->GetClass());
	}
	FreeList->Pool.Push(Action);
}

void USussPoolSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FScopeLock Lock(&Guard);

	// Deallocate anything left
	for (auto& H : FreeArrayPools)
	{
		H.Destroy();
	}
	FreeArrayPools.Empty();
	
	for (auto& H : FreeMapPools)
	{
		H.Destroy();
	}
	FreeMapPools.Empty();

	FreeActionClassPools.Empty();
}
