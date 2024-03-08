// 

#pragma once

#include "CoreMinimal.h"
#include "SussContext.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SussPoolSubsystem.generated.h"

/// Variant typed pointer holder (passed by value)
struct FSussPooledArrayPtr
{
public:
	/// Internal variant pointer
	TVariant<
		TArray<TWeakObjectPtr<AActor>>*,
		TArray<FVector>*,
		TArray<FRotator>*,
		TArray<TSussContextValue>*,
		TArray<FSussContext>*> ArrayPointer;

	FSussPooledArrayPtr(TArray<TWeakObjectPtr<AActor>>* InActors)
	{
		ArrayPointer.Set<TArray<TWeakObjectPtr<AActor>>*>(InActors);
	}
	FSussPooledArrayPtr(TArray<FVector>* InLocations)
	{
		ArrayPointer.Set<TArray<FVector>*>(InLocations);
	}
	FSussPooledArrayPtr(TArray<FRotator>* InRots)
	{
		ArrayPointer.Set<TArray<FRotator>*>(InRots);
	}
	FSussPooledArrayPtr(TArray<TSussContextValue>* InVals)
	{
		ArrayPointer.Set<TArray<TSussContextValue>*>(InVals);
	}
	FSussPooledArrayPtr(TArray<FSussContext>* InContexts)
	{
		ArrayPointer.Set<TArray<FSussContext>*>(InContexts);
	}

	void Destroy()
	{
		// Could do this via TUniquePtr maybe, but then I wouldn't be able to copy this holder around
		if (ArrayPointer.IsType<TArray<TWeakObjectPtr<AActor>>*>())
		{
			delete ArrayPointer.Get<TArray<TWeakObjectPtr<AActor>>*>();
		}
		else if (ArrayPointer.IsType<TArray<FVector>*>())
		{
			delete ArrayPointer.Get<TArray<FVector>*>();
		}
		else if (ArrayPointer.IsType<TArray<FRotator>*>())
		{
			delete ArrayPointer.Get<TArray<FRotator>*>();
		}
		else if (ArrayPointer.IsType<TArray<TSussContextValue>*>())
		{
			delete ArrayPointer.Get<TArray<TSussContextValue>*>();
		}
		else if (ArrayPointer.IsType<TArray<FSussContext>*>())
		{
			delete ArrayPointer.Get<TArray<FSussContext>*>();
		}
	}

	void Reset() const
	{
		// A bit clunky but it's the price we pay for a non-templated holder
		// Use reset not empty to keep allocations
		if (ArrayPointer.IsType<TArray<TWeakObjectPtr<AActor>>*>())
		{
			ArrayPointer.Get<TArray<TWeakObjectPtr<AActor>>*>()->Reset();
		}
		else if (ArrayPointer.IsType<TArray<FVector>*>())
		{
			ArrayPointer.Get<TArray<FVector>*>()->Reset();
		}
		else if (ArrayPointer.IsType<TArray<FRotator>*>())
		{
			ArrayPointer.Get<TArray<FRotator>*>()->Reset();
		}
		else if (ArrayPointer.IsType<TArray<TSussContextValue>*>())
		{
			ArrayPointer.Get<TArray<TSussContextValue>*>()->Reset();
		}
		else if (ArrayPointer.IsType<TArray<FSussContext>*>())
		{
			ArrayPointer.Get<TArray<FSussContext>*>()->Reset();
		}
		
	}
	
};

struct FSussScopeReservedArray
{
protected:
	FSussPooledArrayPtr Holder;
	TWeakObjectPtr<class USussPoolSubsystem> OwningSystem;
public:
	FSussScopeReservedArray(const FSussPooledArrayPtr& H, USussPoolSubsystem* System) : Holder(H), OwningSystem(System) {}
	~FSussScopeReservedArray();

	template<typename T>
	TArray<T>* Get()
	{
		return Holder.ArrayPointer.Get<TArray<T>*>();
	}
};

/**
 * Helper system to provide re-usable pools of eg arrays between brains so they don't have to maintain their own for temp results.
 */
UCLASS()
class USussPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
protected:
	mutable FCriticalSection Guard;

	TArray<FSussPooledArrayPtr> FreeArrayPools;

	template<typename T>
	FSussScopeReservedArray ReserveArrayImpl()
	{
		FScopeLock Lock(&Guard);

		for (int i = 0; i < FreeArrayPools.Num(); ++i)
		{
			if (FreeArrayPools[i].ArrayPointer.IsType<T*>())
			{
				FSussPooledArrayPtr H = FreeArrayPools[i];
				FreeArrayPools.RemoveAt(i);
				return FSussScopeReservedArray(H, this);
			}
		}

		// If we got here, did not exist
		return FSussScopeReservedArray(FSussPooledArrayPtr(new T()), this);
	}

public:

	template<typename T>
	FSussScopeReservedArray ReserveArray()
	{
		return ReserveArrayImpl<TArray<T>>();
	}
	
	void FreeArray(const FSussPooledArrayPtr& Holder)
	{
		FScopeLock Lock(&Guard);

		Holder.Reset();
		FreeArrayPools.Add(Holder);
	}

	virtual void Deinitialize() override;
};

inline USussPoolSubsystem* GetSussArrayPool(UWorld* WorldContext)
{
	if (IsValid(WorldContext) && WorldContext->IsGameWorld())
	{
		auto GI = WorldContext->GetGameInstance();
		if (IsValid(GI))
			return GI->GetSubsystem<USussPoolSubsystem>();		
	}
		
	return nullptr;
}
