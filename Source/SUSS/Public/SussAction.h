// 

#pragma once

#include "CoreMinimal.h"
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
	/// Override this in subclasses if you want this action to not allow interruptions.
	UPROPERTY(EditDefaultsOnly)
	bool bAllowInterruptions = true;

	UPROPERTY(BlueprintReadOnly)
	USussBrainComponent* Brain;

	UPROPERTY(BlueprintReadOnly)
	FSussContext CurrentContext;
public:

	void Init(USussBrainComponent* InBrain, const FSussContext& InContext) { Brain = InBrain; CurrentContext = InContext; }
	
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

	/**
	 * Called when the action has been decided on, and the brain wishes it to be performed.
	 * Override this function to perform the actual underlying actions. Note that action instances are RE-USED,
	 * so be sure to make sure your state is properly re-initialised when this function is called.
	 * Implementations MUST CALL ActionCompleted() at the natural end of the action.
	 * @param Context The context in which the action should be performed.
	 * @param PreviousActionClass If non-null, contains the action class which was previously being performed when this action interrupted it.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void PerformAction(const FSussContext& Context, TSubclassOf<USussAction> PreviousActionClass);

	/// Called when the action has been interrupted because the brain has changed its mind, before completion, or has otherwise been interrupted.
	/// Will only be called if CanBeInterrupted() returns true, otherwise no other action can be performed until
	/// ActionCompleted() is called.
	/// The incoming parameter contains the class of the action which is interrupting this action, if any.
	UFUNCTION(BlueprintNativeEvent)
	void CancelAction(TSubclassOf<USussAction> InterruptedByActionClass);

	/// Subclasses must call this function when they complete their action normally, but not when cancelled.
	UFUNCTION(BlueprintCallable)
	virtual void ActionCompleted();

	/// Implement this to output a series of locations in the gameplay debugger
	UFUNCTION(BlueprintNativeEvent)
	void DebugLocations(UPARAM(Ref) TArray<FVector>& OutLocations, bool bIncludeDetails) const;

	FSussOnActionCompleted InternalOnActionCompleted;
};
