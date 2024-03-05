// 

#pragma once

#include "CoreMinimal.h"
#include "SussActionSet.h"
#include "SussContext.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "SussBrainComponent.generated.h"


/// Output result of an action+context pair being considered; only recorded if score > 0
USTRUCT()
struct FActionScoringResult
{
	GENERATED_BODY()

public:
	FSussActionDef* Def;
	FSussContext Context;
	float Score;
	
};

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

	/// Re-use of pre-defined action sets
	UPROPERTY(EditDefaultsOnly)
	TArray<USussActionSet*> ActionSets;
	
	/// Specific action definitions for this behaviour (if you don't want to re-use from sets)
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussActionDef> ActionDefs;

	float CachedUpdateRequestTime;
	mutable TWeakObjectPtr<AAIController> AiController;

	/// Combination of ActionSets and ActionDefs, sorted by descending priority group
	TArray<FSussActionDef> CombinedActionsByPriority;

	/// The current action being executed, if any
	TOptional<FActionScoringResult> CurrentAction;
	float CurrentActionInertia = 0;
	float CurrentActionInertiaCooldown = 0;
	

	

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
	void InitActions();
	void CheckForNeededUpdate(float DeltaTime);
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UFUNCTION()
	void OnActionCompleted(USussAction* SussAction);
	void ChooseAction(const FActionScoringResult& ActionResult);
	
};
