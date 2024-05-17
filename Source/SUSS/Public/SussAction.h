// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussContext.h"
#include "UObject/Object.h"
#include "SussAction.generated.h"

struct FSussContext;
class USussBrainComponent;

DECLARE_DELEGATE_OneParam(FSussOnActionCompleted, class USussAction*);
/**
 * An Action is one thing that an AI decides to do right now.
 * Actions are instantiated when they're executed, and can therefore store state for the duration of the action.
 * Action instances are RE-USED, so ensure that you reset any state when PerformAction is called.
 */
UCLASS(Blueprintable, Abstract)
class SUSS_API USussAction : public UObject
{
	GENERATED_BODY()
protected:

	/// The tag which identifies the action which this provider is supplying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Suss.Action"))
	FGameplayTag ActionTag;
	
	/// Override this in subclasses if you want this action to not allow interruptions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAllowInterruptions = true;
	/// If interruptions are allowed, set this to true to ONLY allow actions in higher priority groups to interrupt.
	/// Usually actions in the same priority group could interrupt if their scores are higher. Lower priority groups could
	/// also interrupt if the current score for this ongoing action is 0. This flag means regardless of scores only higher
	/// priority actions can interrupt
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAllowInterruptionsFromHigherPriorityGroupsOnly = false;

	UPROPERTY(BlueprintReadOnly)
	USussBrainComponent* Brain;

	UPROPERTY(BlueprintReadOnly)
	FSussContext CurrentContext;

	UPROPERTY(BlueprintReadOnly, Transient)
	int BrainActionIndex;
public:

	void Init(USussBrainComponent* InBrain, const FSussContext& InContext, int ActionIndex)
	{
		Brain = InBrain;
		CurrentContext = InContext;
		BrainActionIndex = ActionIndex;
	}

	const FGameplayTag& GetActionTag() const { return ActionTag; }
	
	virtual UWorld* GetWorld() const override;

	USussBrainComponent* GetBrain() const
	{
		return Brain;
	}

	const FSussContext& GetCurrentContext() const
	{
		return CurrentContext;
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanBeInterrupted() const;

	bool AllowInterruptionsFromHigherPriorityGroupsOnly() const
	{
		return bAllowInterruptionsFromHigherPriorityGroupsOnly;
	}

	/**
	 * Called when the action has been decided on, and the brain wishes it to be performed.
	 * Override this function to perform the actual underlying actions. Note that action instances are RE-USED,
	 * so be sure to make sure your state is properly re-initialised when this function is called.
	 * Implementations MUST CALL ActionCompleted() at the natural end of the action.
	 * @param Context The context in which the action should be performed.
	 * @param Params Optional parameters passed to this action for invocation
	 * @param PreviousActionClass If non-null, contains the action class which was previously being performed when this action interrupted it.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void PerformAction(const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TSubclassOf<USussAction> PreviousActionClass);

	/**
	 * Called when the brain has updated, and the decision is to continue with this action.
	 * You don't necessarily have to do anything here, but it's a helpful "brain frequency" version of Tick() which
	 * you could use to do any updates to the action that don't need to run at frame tick frequency.
	 * @param Context The context (same as the one included in the call to PerformAction)
	 * @param Params Optional parameters (same as those included in the call to PerformAction)
	 */
	UFUNCTION(BlueprintNativeEvent)
	void ContinueAction(const FSussContext& Context, const TMap<FName, FSussParameter>& Params);

	/// Called when the action has been interrupted because the brain has changed its mind, before completion, or has otherwise been interrupted.
	/// Will only be called if CanBeInterrupted() returns true, otherwise no other action can be performed until
	/// ActionCompleted() is called.
	/// The incoming parameter contains the class of the action which is interrupting this action, if any.
	UFUNCTION(BlueprintNativeEvent)
	void CancelAction(TSubclassOf<USussAction> InterruptedByActionClass);

	/// Subclasses must call this function when they complete their action normally, but not when cancelled.
	UFUNCTION(BlueprintCallable)
	virtual void ActionCompleted();

	/// Method that can be implemented in Blueprints to do extra work on action completion called by a C++ base
	UFUNCTION(BlueprintImplementableEvent)
	void OnActionCompleted();

	/// Implement this to output a series of locations in the gameplay debugger
	UFUNCTION(BlueprintNativeEvent)
	void DebugLocations(UPARAM(Ref) TArray<FVector>& OutLocations, bool bIncludeDetails) const;

	/**
	 * Set a temporary score adjustment to this action, so that future evaluations of this action
	 * (with any context) will be temporarily up- or down-voted. While these things should ideally be
	 * scored in considerations, sometimes you might want to just add a temporary "thumb on the scale". 
	 * @param Value The scoring value adjustment to add to the usual score in future. This can be negative if you want to penalise
	 * the running of this action for a while. This will override any previous adjustment
	 * @param CooldownTime The time it should take for this adjustment to slowly reduce back to 0. 
	 */
	UFUNCTION(BlueprintCallable)
	void SetTemporaryActionScoreAdjustment(float Value, float CooldownTime);

	/**
	 * Add a temporary score adjustment to this action, for this brain, so that future evaluations of this action
	 * (with any context) will be temporarily up- or down-voted. This could be useful if there are conditions you might
	 * encounter while executing this action that should alter its likelihood to be chosen again. While you could put these
	 * things in considerations, sometimes that's not practical because it requires more information specific to the
	 * action execution. This allows an action to essentially "veto" itself for a while, by calling this function and
	 * then ActionCompleted() to trigger the re-evaluation of alternatives.
	 * @param Value The scoring value to add to the usual score in future. This can be negative if you want to penalise
	 * the running of this action for a while (on this brain).
	 * @param CooldownTime The time it should take for this adjustment to slowly reduce back to 0. If set to 0, this
	 * score adjustment will not be removed until manually reset.
	 */
	UFUNCTION(BlueprintCallable)
	void AddTemporaryScoreAdjustment(float Value, float CooldownTime);

	/// Resets any score adjustments added via AddTemporaryScoreAdjustment immediately
	UFUNCTION(BlueprintCallable)
	void ResetTemporaryScoreAdjustment();

	FSussOnActionCompleted InternalOnActionCompleted;
};
