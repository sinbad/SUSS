
#include "SussAction.h"

void USussAction::PerformAction_Implementation(USussBrainComponent* Brain, const FSussContext& Context)
{
	// Subclasses must implement
}

void USussAction::CancelAction_Implementation(USussBrainComponent* Brain, const FSussContext& Context)
{
	// Subclasses must implement
}

bool USussAction::CanBeInterrupted_Implementation() const
{
	return bAllowInterruptions;
}

void USussAction::ActionCompleted()
{
	InternalOnActionCompleted.ExecuteIfBound(this);
}
