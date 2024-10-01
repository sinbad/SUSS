// 

#pragma once

#include "CoreMinimal.h"
#include "SussContext.h"
#include "SussParameter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SussPoolSubsystem.generated.h"

/// Variant typed pointer holder (passed by value)
struct FSussPooledArrayPtr
{
protected:
	bool bIsBound;
	/// Internal variant pointer
	TVariant<
		TArray<TWeakObjectPtr<AActor>>*,
		TArray<FVector>*,
		TArray<FRotator>*,
		TArray<FGameplayTag>*,
		TArray<FSussContextValue>*,
		TArray<FSussContext>*> ArrayPointer;

public:
	FSussPooledArrayPtr() : bIsBound(false) {}
	FSussPooledArrayPtr(TArray<TWeakObjectPtr<AActor>>* InActors) : bIsBound(true)
	{
		ArrayPointer.Set<TArray<TWeakObjectPtr<AActor>>*>(InActors);
	}
	FSussPooledArrayPtr(TArray<FVector>* InLocations): bIsBound(true)
	{
		ArrayPointer.Set<TArray<FVector>*>(InLocations);
	}
	FSussPooledArrayPtr(TArray<FRotator>* InRots): bIsBound(true)
	{
		ArrayPointer.Set<TArray<FRotator>*>(InRots);
	}
	FSussPooledArrayPtr(TArray<FGameplayTag>* InTags): bIsBound(true)
	{
		ArrayPointer.Set<TArray<FGameplayTag>*>(InTags);
	}
	FSussPooledArrayPtr(TArray<FSussContextValue>* InVals): bIsBound(true)
	{
		ArrayPointer.Set<TArray<FSussContextValue>*>(InVals);
	}
	FSussPooledArrayPtr(TArray<FSussContext>* InContexts): bIsBound(true)
	{
		ArrayPointer.Set<TArray<FSussContext>*>(InContexts);
	}

	void Bind(TArray<TWeakObjectPtr<AActor>>* InActors)
	{
		ArrayPointer.Set<TArray<TWeakObjectPtr<AActor>>*>(InActors);
		bIsBound = true;
	}
	void Bind(TArray<FVector>* InLocations)
	{
		ArrayPointer.Set<TArray<FVector>*>(InLocations);
		bIsBound = true;
	}
	void Bind(TArray<FRotator>* InRots)
	{
		ArrayPointer.Set<TArray<FRotator>*>(InRots);
		bIsBound = true;
	}
	void Bind(TArray<FGameplayTag>* InTags)
	{
		ArrayPointer.Set<TArray<FGameplayTag>*>(InTags);
		bIsBound = true;
	}
	void Bind(TArray<FSussContextValue>* InVals)
	{
		ArrayPointer.Set<TArray<FSussContextValue>*>(InVals);
		bIsBound = true;
	}
	void Bind(TArray<FSussContext>* InContexts)
	{
		ArrayPointer.Set<TArray<FSussContext>*>(InContexts);
		bIsBound = true;
	}
	

	void Destroy()
	{
		if (bIsBound)
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
			else if (ArrayPointer.IsType<TArray<FGameplayTag>*>())
			{
				delete ArrayPointer.Get<TArray<FGameplayTag>*>();
			}
			else if (ArrayPointer.IsType<TArray<FSussContextValue>*>())
			{
				delete ArrayPointer.Get<TArray<FSussContextValue>*>();
			}
			else if (ArrayPointer.IsType<TArray<FSussContext>*>())
			{
				delete ArrayPointer.Get<TArray<FSussContext>*>();
			}
			bIsBound = false;
		}
	}

	void Reset() const
	{
		if (bIsBound)
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
			else if (ArrayPointer.IsType<TArray<FGameplayTag>*>())
			{
				ArrayPointer.Get<TArray<FGameplayTag>*>()->Reset();
			}
			else if (ArrayPointer.IsType<TArray<FSussContextValue>*>())
			{
				ArrayPointer.Get<TArray<FSussContextValue>*>()->Reset();
			}
			else if (ArrayPointer.IsType<TArray<FSussContext>*>())
			{
				ArrayPointer.Get<TArray<FSussContext>*>()->Reset();
			}
		}
		
	}

	template<typename T>
	TArray<T>* Get() const
	{
		if (bIsBound)
		{
			return ArrayPointer.Get<TArray<T>*>();
		}
		return nullptr;
	}

	template<typename T>
	bool ContainsType() const
	{
		if (bIsBound)
		{
			return ArrayPointer.IsType<TArray<T>*>();
		}
		return false;
	}
	
};

struct FSussScopeReservedArray
{
protected:
	FSussPooledArrayPtr Holder;
	TWeakObjectPtr<class USussPoolSubsystem> OwningSystem;
public:
	FSussScopeReservedArray()
	{
	}

	FSussScopeReservedArray(const FSussPooledArrayPtr& H, USussPoolSubsystem* System) : Holder(H), OwningSystem(System) {}
	~FSussScopeReservedArray();

	template<typename T>
	TArray<T>* Get()
	{
		return Holder.Get<T>();
	}
	
	FSussScopeReservedArray(FSussScopeReservedArray&& Other) noexcept
		: Holder(std::move(Other.Holder)),
		  OwningSystem(std::move(Other.OwningSystem))
	{
	}
	
	FSussScopeReservedArray& operator=(FSussScopeReservedArray&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		Holder = std::move(Other.Holder);
		OwningSystem = std::move(Other.OwningSystem);
		return *this;
	}
};

struct FSussPooledMapPtr
{
protected:
	/// Internal variant pointer
	TVariant<
		TMap<FName, FSussParameter>*,
		TMap<FName, FSussContextValue>*,
		TMap<FName, FSussScopeReservedArray>*> MapPointer;

public:
	FSussPooledMapPtr(TMap<FName, FSussParameter>* Params)
	{
		MapPointer.Set<TMap<FName, FSussParameter>*>(Params);
	}
	FSussPooledMapPtr(TMap<FName, FSussContextValue>* Params)
	{
		MapPointer.Set<TMap<FName, FSussContextValue>*>(Params);
	}
	FSussPooledMapPtr(TMap<FName, FSussScopeReservedArray>* Params)
	{
		MapPointer.Set<TMap<FName, FSussScopeReservedArray>*>(Params);
	}
	

	void Destroy()
	{
		// Could do this via TUniquePtr maybe, but then I wouldn't be able to copy this holder around
		if (MapPointer.IsType<TMap<FName, FSussParameter>*>())
		{
			delete MapPointer.Get<TMap<FName, FSussParameter>*>();
		}
		else if (MapPointer.IsType<TMap<FName, FSussContextValue>*>())
		{
			delete MapPointer.Get<TMap<FName, FSussContextValue>*>();
		}
		else if (MapPointer.IsType<TMap<FName, FSussScopeReservedArray>*>())
		{
			delete MapPointer.Get<TMap<FName, FSussScopeReservedArray>*>();
		}
	}

	void Reset() const
	{
		// A bit clunky but it's the price we pay for a non-templated holder
		// Use reset not empty to keep allocations
		if (MapPointer.IsType<TMap<FName, FSussParameter>*>())
		{
			MapPointer.Get<TMap<FName, FSussParameter>*>()->Reset();
		}
		else if (MapPointer.IsType<TMap<FName, FSussContextValue>*>())
		{
			MapPointer.Get<TMap<FName, FSussContextValue>*>()->Reset();
		}
		else if (MapPointer.IsType<TMap<FName, FSussScopeReservedArray>*>())
		{
			MapPointer.Get<TMap<FName, FSussScopeReservedArray>*>()->Reset();
		}
	}

	template<typename K, typename V>
	TMap<K,V>* Get() const
	{
		return MapPointer.Get<TMap<K,V>*>();
	}

	template<typename K, typename V>
	bool ContainsType() const
	{
		return MapPointer.IsType<TMap<K,V>*>();
	}

};

struct FSussScopeReservedMap
{
protected:
	FSussPooledMapPtr Holder;
	TWeakObjectPtr<class USussPoolSubsystem> OwningSystem;
public:
	FSussScopeReservedMap(const FSussPooledMapPtr& H, USussPoolSubsystem* System) : Holder(H), OwningSystem(System) {}
	~FSussScopeReservedMap();

	template<typename K, typename V>
	TMap<K, V>* Get()
	{
		return Holder.Get<K, V>();
	}
	
	FSussScopeReservedMap(FSussScopeReservedMap&& Other) noexcept
		: Holder(std::move(Other.Holder)),
		  OwningSystem(std::move(Other.OwningSystem))
	{
	}

	FSussScopeReservedMap& operator=(FSussScopeReservedMap&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		Holder = std::move(Other.Holder);
		OwningSystem = std::move(Other.OwningSystem);
		return *this;
	}
};

// Because we can't have maps of arrays or arrays of arrays
USTRUCT()
struct FSussActionPool
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<class USussAction*> Pool;
};

// Used to auto-free actions
typedef TSharedPtr<USussAction, ESPMode::ThreadSafe> TSussReservedActionPtr;


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
	TArray<FSussPooledMapPtr> FreeMapPools;

	UPROPERTY()
	TMap<UClass*, FSussActionPool> FreeActionClassPools;
	// For GC purposes
	UPROPERTY()
	TMap<UClass*, FSussActionPool> ReservedActionClassPools;
	
	template<typename T>
	FSussScopeReservedArray ReserveArrayImpl()
	{
		FScopeLock Lock(&Guard);

		for (int i = 0; i < FreeArrayPools.Num(); ++i)
		{
			if (FreeArrayPools[i].ContainsType<T>())
			{
				FSussPooledArrayPtr H = FreeArrayPools[i];
				FreeArrayPools.RemoveAt(i);
				return FSussScopeReservedArray(H, this);
			}
		}

		// If we got here, did not exist
		TArray<T>* NewItem = new TArray<T>();
		return FSussScopeReservedArray(FSussPooledArrayPtr(NewItem), this);
	}
	
	template<typename K, typename V>
	FSussScopeReservedMap ReserveMapImpl()
	{
		FScopeLock Lock(&Guard);

		for (int i = 0; i < FreeMapPools.Num(); ++i)
		{
			if (FreeMapPools[i].ContainsType<K,V>())
			{
				FSussPooledMapPtr H = FreeMapPools[i];
				FreeMapPools.RemoveAt(i);
				return FSussScopeReservedMap(H, this);
			}
		}

		// If we got here, did not exist
		TMap<K,V>* NewItem = new TMap<K,V>();
		return FSussScopeReservedMap(FSussPooledMapPtr(NewItem), this);
	}

public:

	template<typename T>
	FSussScopeReservedArray ReserveArray()
	{
		return ReserveArrayImpl<T>();
	}
	
	void FreeArray(const FSussPooledArrayPtr& Holder)
	{
		FScopeLock Lock(&Guard);

		Holder.Reset();
		FreeArrayPools.Add(Holder);
	}

	template<typename K, typename V>
	FSussScopeReservedMap ReserveMap()
	{
		return ReserveMapImpl<K, V>();
	}
	
	void FreeMap(const FSussPooledMapPtr& Holder)
	{
		FScopeLock Lock(&Guard);

		Holder.Reset();
		FreeMapPools.Add(Holder);
	}

	TSussReservedActionPtr ReserveAction(UClass* ActionClass, UObject* TemplateIfCreated);

	// Not neededby clients, just clear your TSussReservedActionPtr
	void InternalFreeAction(USussAction* Action);
	virtual void Deinitialize() override;
};

inline USussPoolSubsystem* GetSussPool(UWorld* WorldContext)
{
	if (IsValid(WorldContext))
	{
		auto GI = WorldContext->GetGameInstance();
		if (IsValid(GI))
			return GI->GetSubsystem<USussPoolSubsystem>();		
	}
		
	return nullptr;
}
