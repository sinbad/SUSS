// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SussAction.generated.h"

class USussBrainComponent;
/**
 * An Action is one thing that an AI decides to do right now.
 * Actions are stateless; they should only change things on other classes such as the AI controller or the pawn.
 */
UCLASS(Blueprintable, Abstract)
class SUSS_API USussAction : public UObject
{
	GENERATED_BODY()

public:

	/// Called when the action has been decided on, and the brain wishes it to be performed.
	/// Override this function to perform the actual underlying actions.
	UFUNCTION(BlueprintNativeEvent)
	void PerformAction(USussBrainComponent* Brain) const;

	/// Called when the action has been interrupted because the brain has changed its mind, before completion
	UFUNCTION(BlueprintNativeEvent)
	void CancelAction(USussBrainComponent* Brain) const;

	

};
