
#pragma once

#include "CoreMinimal.h"
#include "SussActionSetAsset.h"
#include "SussContext.h"
#include "SussPoolSubsystem.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "SussBrainComponent.generated.h"

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
};

/// Output result of an action+context pair being considered; only recorded if score > 0
USTRUCT()
struct FSussActionScoringResult
{
	GENERATED_BODY()

public:
	const FSussActionDef* Def;
	FSussContext Context;
	float Score = 0;

	/// Action instance, only used for actions which are run
	UPROPERTY()
	USussAction* ActionInstance = nullptr;

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

	/// Current brain configuration; can be set in defaults, or imported from USussBrainConfigAsset, or set at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetBrainConfig)
	FSussBrainConfig BrainConfig;

	/// Asset which will be used to provide the brain config. For if you want to pre-author these and simply link them to brains,
	/// either in your Blueprint definition, or later (e.g. AIs using a shared controller class & data-driven brain config).
	/// This will OVERRIDE any existing BrainConfig values at startup.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetBrainConfigFromAsset)
	USussBrainConfigAsset* BrainConfigAsset;

	float CachedUpdateRequestTime;
	mutable TWeakObjectPtr<AAIController> AiController;

	/// Combination of ActionSets and ActionDefs, sorted by descending priority group
	TArray<FSussActionDef> CombinedActionsByPriority;

	/// The current action being executed, if any
	UPROPERTY()	
	FSussActionScoringResult CurrentAction;
	float CurrentActionInertia = 0;
	float CurrentActionInertiaCooldown = 0;

	TArray<FSussActionScoringResult> CandidateActions;
	/// Record of when actions were last run, by class name
	TMap<FName, double> ActionNamesTimeLastPerformed;

public:
	// Sets default values for this component's properties
	USussBrainComponent();

	virtual FString GetDebugInfoString() const override;

	const FSussBrainConfig& GetBrainConfig() const { return BrainConfig; }

	UFUNCTION(BlueprintCallable)
	void SetBrainConfig(const FSussBrainConfig& NewConfig);

	UFUNCTION(BlueprintCallable)
	void SetBrainConfigFromAsset(USussBrainConfigAsset* Asset);

	/// Stop doing any current AI action
	UFUNCTION(BlueprintCallable)
	void StopCurrentAction();

	/// Are we waiting for an update (should be queued already)
	bool NeedsUpdate() const { return bQueuedForUpdate; }
	/// Update function which triggers an evaluation & action decision
	void Update();

	/// Get the AI controller associated with the actor that owns this brain
	UFUNCTION(BlueprintCallable)
	AAIController* GetAIController() const;

	/// Get the "Self" pawn this brain controls, used in contexts
	AActor* GetSelf() const;

	/// Get the time in seconds since an action was last performed
	UFUNCTION(BlueprintCallable)
	double GetTimeSinceActionPerformed(TSubclassOf<USussAction> ActionClass) const;


	virtual void StartLogic() override;
	virtual void RestartLogic() override;
	virtual void StopLogic(const FString& Reason) override;
	virtual void PauseLogic(const FString& Reason) override;
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;

	void DebugLocations(TArray<FVector>& OutLocations) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void BrainConfigChanged();
	void InitActions();
	void CheckForNeededUpdate(float DeltaTime);
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UFUNCTION()
	void OnActionCompleted(USussAction* SussAction);
	void ChooseActionFromCandidates();
	void ChooseAction(const FSussActionScoringResult& ActionResult);
	void CancelCurrentAction(TSubclassOf<USussAction> Interrupter);

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
	static void AppendContexts(AActor* Self, FSussScopeReservedArray& ReservedArray, TArray<FSussContext>& OutContexts, TFunctionRef<void(const T&, FSussContext&)> ValueSetter)
	{
		AppendContexts<T>(Self, *ReservedArray.Get<T>(), OutContexts, ValueSetter);
	}

	void GenerateContexts(AActor* Self, const FSussActionDef& Action, TArray<FSussContext>& OutContexts);
	float ResolveParameterToFloat(const FSussContext& SelfContext, const FSussParameter& Value) const;
	FSussParameter ResolveParameter(const FSussContext& SelfContext, const FSussParameter& Value) const;
	void ResolveParameters(AActor* Self, const TMap<FName, FSussParameter>& InParams, TMap<FName, FSussParameter>& OutParams);
};
