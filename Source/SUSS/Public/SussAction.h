// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SussAction.generated.h"

struct FSussContext;
class USussBrainComponent;

DECLARE_DELEGATE_OneParam(FSussOnActionCompleted, class USussAction*);
/**
 * An Action is one thing that an AI decides to do right now.
 * Actions are stateless; they should only change things on other classes such as the AI controller or the pawn.
 */
UCLASS(Blueprintable, Abstract)
class SUSS_API USussAction : public UObject
{
	GENERATED_BODY()
protected:
	/// Override this in subclasses if you want this action to not allow interruptions.
	UPROPERTY(EditDefaultsOnly)
	bool bAllowInterruptions = true;
public:

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanBeInterrupted() const;

	/// Called when the action has been decided on, and the brain wishes it to be performed.
	/// Override this function to perform the actual underlying actions.
	/// Implementations MUST CALL ActionCompleted() at the natural end of the action.
	UFUNCTION(BlueprintNativeEvent)
	void PerformAction(USussBrainComponent* Brain, const FSussContext& Context) const;

	/// Called when the action has been interrupted because the brain has changed its mind, before completion.
	/// Will only be called if CanBeInterrupted() returns true, otherwise no other action can be performed until
	/// ActionCompleted() is called.
	UFUNCTION(BlueprintNativeEvent)
	void CancelAction(USussBrainComponent* Brain, const FSussContext& Context) const;

	UFUNCTION(BlueprintCallable)
	virtual void ActionCompleted();

	FSussOnActionCompleted InternalOnActionCompleted;
};
