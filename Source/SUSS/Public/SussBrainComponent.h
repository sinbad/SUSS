
#pragma once

#include "CoreMinimal.h"
#include "SussActionSetAsset.h"
#include "SussContext.h"
#include "SussGameSubsystem.h"
#include "SussPoolSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "SussBrainComponent.generated.h"

class UCharacterMovementComponent;
/// How to choose the action to run
UENUM(BlueprintType)
enum class ESussActionChoiceMethod : uint8
{
	/// Always pick the highest scoring action
	HighestScoring,
	/// Pick a weighted random action from all non-zero scoring actions
	WeightedRandomAll,
	/// Pick a weighted random action from the top N non-zero scoring actions
	WeightedRandomTopN,
	/// Pick a weighted random action from all actions scoring within N percent of the top scorer (only non-zero)
	WeightedRandomTopNPercent
};

/// Distance category to any player, determines how quickly a brain ticks
UENUM(BlueprintType)
enum class ESussDistanceCategory : uint8
{
	Near,
	MidRange,
	Far,
	OutOfRange
};

/// Collected configuration for a brain which can be plugged in as needed
USTRUCT(BlueprintType)
struct FSussBrainConfig
{
	GENERATED_BODY()
public:
	/// Re-use of pre-defined action sets
	UPROPERTY(EditDefaultsOnly)
	TArray<USussActionSetAsset*> ActionSets;
	
	/// Specific action definitions for this behaviour (if you don't want to re-use from sets)
	UPROPERTY(EditDefaultsOnly)
	TArray<FSussActionDef> ActionDefs;

	/// How to choose the action to take
	UPROPERTY(EditDefaultsOnly)
	ESussActionChoiceMethod ActionChoiceMethod = ESussActionChoiceMethod::HighestScoring;

	/// When using the "Top N" or "Top N Percent" action choice methods, the value of "N"
	UPROPERTY(EditDefaultsOnly)
	int ActionChoiceTopN = 5;

	/// If any of these gameplay tags exist on the pawn being controlled by the brain, the brain will not be
	/// updated. This is a simple way to avoid new actions being performed while an enemy is stunned, or staggered (this
	/// is easier than checking in every action for this condition).
	/// When none of these tags exist anymore the brain will perform an immediate update if any were queued
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer PreventBrainUpdateIfAnyTags;
};

/// Output result of an action+context pair being considered; only recorded if score > 0
USTRUCT()
struct FSussActionScoringResult
{
	GENERATED_BODY()

public:
	int ActionDefIndex;
	FSussContext Context;
	float Score = 0;
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
	/// Whether this brain is awaiting an update that has been queued with the subsystem
	UPROPERTY(BlueprintReadOnly)
	bool bQueuedForUpdate;

	/// Whether this brain wanted to update, but couldn't because of a condition
	UPROPERTY(BlueprintReadOnly)
	bool bWasPreventedFromUpdating;

	/// Current brain configuration; can be set in defaults, or imported from USussBrainConfigAsset, or set at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetBrainConfig)
	FSussBrainConfig BrainConfig;

	/// Asset which will be used to provide the brain config. For if you want to pre-author these and simply link them to brains,
	/// either in your Blueprint definition, or later (e.g. AIs using a shared controller class & data-driven brain config).
	/// This will OVERRIDE any existing BrainConfig values at startup.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetBrainConfigFromAsset)
	USussBrainConfigAsset* BrainConfigAsset;

	UPROPERTY(BlueprintReadOnly)
	ESussDistanceCategory DistanceCategory;

	/// The timer that handles the update requests (and also checks distance).
	/// This runs at a variable rate depending on distance to players.
	FTimerHandle UpdateRequestTimer;
	float CurrentUpdateInterval;

	mutable TWeakObjectPtr<AAIController> AiController;

	/// Combination of ActionSets and ActionDefs, sorted by descending priority group
	TArray<FSussActionDef> CombinedActionsByPriority;

	/// The scoring result of the current action definition being executed, if any
	FSussActionScoringResult CurrentActionResult;
	/// The instance of the action being executed
	UPROPERTY()
	USussAction* CurrentActionInstance = nullptr;
	float CurrentActionInertia = 0;
	float CurrentActionInertiaCooldown = 0;

	TArray<FSussActionScoringResult> CandidateActions;
	/// Record of when actions were last run, by class name
	TMap<FGameplayTag, double> ActionsTimeLastPerformed;

	UPROPERTY(Transient)
	UAIPerceptionComponent* PerceptionComp;
	TMap<FGameplayTag, FDelegateHandle> TagDelegates;

public:
	// Sets default values for this component's properties
	USussBrainComponent();

	const FSussBrainConfig& GetBrainConfig() const { return BrainConfig; }

	UFUNCTION(BlueprintCallable)
	void SetBrainConfig(const FSussBrainConfig& NewConfig);

	UFUNCTION(BlueprintCallable)
	void SetBrainConfigFromAsset(USussBrainConfigAsset* Asset);

	/// Stop doing any current AI action
	UFUNCTION(BlueprintCallable)
	void StopCurrentAction();

	ESussDistanceCategory GetDistanceCategory() const { return DistanceCategory; }

	/// Are we waiting for an update (should be queued already)
	bool NeedsUpdate() const { return bQueuedForUpdate; }
	/// Update function which triggers an evaluation & action decision
	void Update();

	/// Get the AI controller associated with the actor that owns this brain
	UFUNCTION(BlueprintCallable)
	AAIController* GetAIController() const;

	/// Get the character movement component associated with the actor that owns this brain, if any.
	/// This is for convenience
	UFUNCTION(BlueprintCallable)
	UCharacterMovementComponent* GetCharacterMovement() const;

	/// Get the pawn being controlled by this brain, for convenience
	/// Function / display name for consistency with AIController
	UFUNCTION(BlueprintCallable, meta=(DisplayName="Get Controlled Pawn"))
	APawn* GetPawn() const;

	/// Get the "Self" pawn this brain controls, used in contexts
	AActor* GetSelf() const;

	/// Retrieve the perception component
	UFUNCTION(BlueprintCallable)
	UAIPerceptionComponent* GetPerceptionComponent() const { return PerceptionComp; }

	/// Retrieve the blackboard
	UFUNCTION(BlueprintCallable)
	UBlackboardComponent* GetBlackboard() const { return BlackboardComp; }
	
	/// Get the time in seconds since an action was last performed
	UFUNCTION(BlueprintCallable)
	double GetTimeSinceActionPerformed(FGameplayTag ActionTag) const;

	/// Request an update to this brain, outside the usual update interval
	UFUNCTION(BlueprintCallable)
	void RequestUpdate();



	virtual void StartLogic() override;
	virtual void RestartLogic() override;
	virtual void StopLogic(const FString& Reason) override;
	virtual void PauseLogic(const FString& Reason) override;
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;

	// NOT GetDebugInfoString, since that makes it duplicate in BehaviourTree category
	FString GetDebugSummaryString() const;
	void DebugLocations(TArray<FVector>& OutLocations, bool bIncludeDetails) const;
	void GetDebugDetailLines(TArray<FString>& OutLines) const;

	UFUNCTION(BlueprintCallable)
	bool IsActionInProgress();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void BrainConfigChanged();
	void InitActions();
	void QueueForUpdate();
	void TimerCallback();
	float GetDistanceToAnyPlayer() const;
	void UpdateDistanceCategory();
	bool IsUpdatePrevented() const;

	UFUNCTION()
	void OnActionCompleted(USussAction* SussAction);
	void ChooseActionFromCandidates();
	void ChooseAction(const FSussActionScoringResult& ActionResult);
	void CancelCurrentAction(TSubclassOf<USussAction> Interrupter);
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& Actors);
	UFUNCTION()
	void OnGameplayTagEvent(const FGameplayTag InTag, int32 NewCount);

	template<typename T>
	static void AppendUncorrelatedContexts(AActor* Self, const TArray<T>& InValues, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
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
				OutContext.ControlledActor = Self;
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
	static void AppendUncorrelatedContexts(AActor* Self, FSussScopeReservedArray& ReservedArray, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		AppendUncorrelatedContexts<T>(Self, *ReservedArray.Get<T>(), OutContexts, ValueSetter);
	}

	template<typename T>
	static void AppendCorrelatedContexts(AActor* Self, const TArray<T>& InValues, FSussContext& SourceContext, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		if (InValues.IsEmpty())
			return;

		// The first value from InValues is combined with the SourceContext
		// Every other value generates a new copy of SourceContext with that value added
		// So we expand 1 SourceContext to 1..N contexts with the InValues set

		ValueSetter(InValues[0], SourceContext);

		const int OldSize = OutContexts.Num();
		const int InValueCount = InValues.Num();
		if (InValueCount > 1)
		{
			OutContexts.AddDefaulted(InValueCount - 1);

			int OutIndex = OldSize;
			for (int i = 1; i < InValues.Num(); ++i, ++OutIndex)
			{
				FSussContext* OutContext = &OutContexts[OutIndex];
				// Init all values, then set new one
				*OutContext = SourceContext;

				ValueSetter(InValues[i], *OutContext);

			}
		}
	}
	template<typename T>
	static void AppendCorrelatedContexts(AActor* Self, FSussScopeReservedArray& ReservedArray, FSussContext& SourceContext, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		AppendCorrelatedContexts<T>(Self, *ReservedArray.Get<T>(), SourceContext, OutContexts, ValueSetter);
	}

	void GenerateContexts(AActor* Self, const FSussActionDef& Action, TArray<FSussContext>& OutContexts);
	void IntersectCorrelatedContexts(AActor* Self, const FSussQuery& Query, USussQueryProvider* QueryProvider, const TMap<FName, FSussParameter>& Params, TArray<FSussContext>& InOutContexts);
	void AppendUncorrelatedContexts(AActor* Self, const FSussQuery& Query, USussQueryProvider* QueryProvider, const TMap<FName, FSussParameter>& Params, TArray<FSussContext>& OutContexts);
	FSussParameter ResolveParameter(const FSussContext& SelfContext, const FSussParameter& Value) const;
	void ResolveParameters(AActor* Self, const TMap<FName, FSussParameter>& InParams, TMap<FName, FSussParameter>& OutParams);
};
