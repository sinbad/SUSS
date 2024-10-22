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

TSussReservedActionPtr USussPoolSubsystem::ReserveAction(UClass* ActionClass,
                                                         UObject* TemplateIfCreated)
{
	FScopeLock Lock(&Guard);

	USussAction* Ret = nullptr;
	if (auto FreeList = FreeActionClassPools.Find(ActionClass))
	{
		if (FreeList->Pool.Num() > 0)
		{
			Ret = FreeList->Pool.Pop();
		}
	}

	if (!Ret)
	{
		Ret = NewObject<USussAction>(this, ActionClass, NAME_None, RF_NoFlags, TemplateIfCreated);
	}

	auto ReserveList = ReservedActionClassPools.Find(ActionClass);
	if (!ReserveList)
	{
		ReserveList = &ReservedActionClassPools.Emplace(ActionClass);
	}
	ReserveList->Pool.Push(Ret);

	//UE_LOG(LogTemp, Warning, TEXT("Reserved action: %s"), *Ret->GetName())

	// return a shared ptr which doesn't delete, it frees the reserved action
	// Use WeakPtr to guard against shutdown issues
	TWeakObjectPtr<USussPoolSubsystem> WeakThis(this);
	return TSussReservedActionPtr(Ret, [WeakThis](USussAction* Obj)
	{
		if (Obj && WeakThis.IsValid()) 
		{
			WeakThis->InternalFreeAction(Obj);
		}
	});

}

void USussPoolSubsystem::InternalFreeAction(USussAction* Action)
{
	FScopeLock Lock(&Guard);

	if (!Action)
		return;

	//UE_LOG(LogTemp, Warning, TEXT("Freed action: %s"), *Action->GetName())
	
	auto FreeList = FreeActionClassPools.Find(Action->GetClass());
	if (!FreeList)
	{
		FreeList = &FreeActionClassPools.Emplace(Action->GetClass());
	}
	FreeList->Pool.Push(Action);

	// Remove from reserve
	if (auto ReserveList = ReservedActionClassPools.Find(Action->GetClass()))
	{
		ReserveList->Pool.Remove(Action);
	}

	// Unbind any callback if back to pool
	Action->InternalOnActionCompleted.Unbind();
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
