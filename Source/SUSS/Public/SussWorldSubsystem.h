// 

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Subsystems/WorldSubsystem.h"
#include "SussWorldSubsystem.generated.h"

class USussBrainComponent;
/**
 * World-scope subsystem used to manage brains which need updating.
 */
UCLASS()
class SUSS_API USussWorldSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
protected:
	/// The max time brains are allowed to take to update in a frame. Once this time is gone, brains still needing update
	/// will be scheduled for the next frame
	float CachedFrameTimeBudgetMs;

	/// Brains which need updating, FIFO
	TQueue<TWeakObjectPtr<USussBrainComponent>> BrainsToUpdate;

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	void UpdateBrains();

public:

	USussWorldSubsystem();
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	

	/// Queue a brain to be updated
	void QueueBrainUpdate(USussBrainComponent* Brain);

	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
};

inline USussWorldSubsystem* GetSussWorldSubsystem(const UWorld* World)
{
	if (IsValid(World) && World->IsGameWorld())
	{
		return World->GetSubsystem<USussWorldSubsystem>();
	}
		
	return nullptr;
}

