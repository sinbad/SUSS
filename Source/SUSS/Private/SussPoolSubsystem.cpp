// 


#include "SussPoolSubsystem.h"

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
}
