// 

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "SussBrainComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SUSS_API USussBrainComponent : public UBrainComponent
{
	GENERATED_BODY()

protected:
	/// Whether this brain is awaiting an update
	UPROPERTY(BlueprintReadOnly)
	bool bQueuedForUpdate;

	/// Time that has elapsed since the last brain update
	UPROPERTY(BlueprintReadOnly)
	float TimeSinceLastUpdate;

	float CachedUpdateRequestTime;
	mutable TWeakObjectPtr<AAIController> AiController;

public:
	// Sets default values for this component's properties
	USussBrainComponent();

	/// Are we waiting for an update (should be queued already)
	bool NeedsUpdate() const { return bQueuedForUpdate; }
	/// Update function which triggers an evaluation & action decision
	void Update();

	/// Get the AI controller associated with the actor that owns this brain
	UFUNCTION(BlueprintCallable)
	AAIController* GetAIController() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
};
