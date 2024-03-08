
#pragma once

#include "CoreMinimal.h"
#include "SussActionSet.h"
#include "SussContext.h"
#include "SussPoolSubsystem.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "SussBrainComponent.generated.h"


/// Output result of an action+context pair being considered; only recorded if score > 0
USTRUCT()
struct FSussActionScoringResult
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

#if WITH_AUTOMATION_TESTS
public:
	friend class FSussBrainTestContextsSpec;
#endif
	
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
	TOptional<FSussActionScoringResult> CurrentAction;
	float CurrentActionInertia = 0;
	float CurrentActionInertiaCooldown = 0;

	TArray<FSussActionScoringResult> CandidateActions;


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

	/// Get the "Self" pawn this brain controls, used in contexts
	AActor* GetSelf() const;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void InitActions();
	void CheckForNeededUpdate(float DeltaTime);
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UFUNCTION()
	void OnActionCompleted(USussAction* SussAction);
	void ChooseActionFromCandidates(const TArray<FSussActionScoringResult>& Candidates);
	void ChooseAction(const FSussActionScoringResult& ActionResult);

	template<typename T>
	static void AppendContexts(AActor* Self, const TArray<T>& InValues, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		if (InValues.IsEmpty())
			return;

		const int OldSize = OutContexts.Num();

		if (OldSize == 0)
		{
			// This is the first set of values, so simply copy them in
			OutContexts.SetNum(InValues.Num());
			for (int i = 0; i < InValues.Num(); ++i)
			{
				FSussContext& OutContext = OutContexts[i];
				OutContext.Self = Self;
				ValueSetter(InValues[i], OutContext);
			}
		}
		else
		{
			// Add all combinations of old & new
			OutContexts.SetNum(OldSize * InValues.Num());

			int OutIndex = 0;

			// Outer loop is for all the incoming new values
			// We need to repeat the existing entries InValues.Num() times to create all combinations
			// The first OldSize entries already have the other fields populated; all the others need to be initialised from
			// the values from OldSize before adding the new InValue combinations
			for (int i = 0; i < InValues.Num(); ++i)
			{
				for (int j = 0; j < OldSize; ++j, ++OutIndex)
				{
					FSussContext* OutContext = &OutContexts[OutIndex];
					if (i > 0)
					{
						// This is the second+ entry, which means we need to populate the other fields from the first OldSize values
						// If we're within the first loop, we're still on the OldSize entries which already contain everything except the new values
						*OutContext = OutContexts[j];
					}
				
					// Now we need to add the InValue combination
					ValueSetter(InValues[i], *OutContext);
				}
			}
		}
	}
	template<typename T>
	static void AppendContexts(AActor* Self, FSussScopeReservedArray& ReservedArray, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		AppendContexts<T>(Self, *ReservedArray.Get<T>(), OutContexts, ValueSetter);
	}

	void GenerateContexts(const FSussActionDef& Action, TArray<FSussContext>& OutContexts);
	FSussParameter ResolveParameter(const FSussContext& SelfContext, const FSussParameter& Value) const;
	void ResolveParameters(AActor* Self, const TMap<FName, FSussParameter>& InParams, TMap<FName, FSussParameter>& OutParams);
	
};
