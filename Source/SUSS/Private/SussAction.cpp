
#include "SussAction.h"

void USussAction::PerformAction_Implementation(USussBrainComponent* Brain, const FSussContext& Context) const
{
	// Subclasses must implement
}

void USussAction::CancelAction_Implementation(USussBrainComponent* Brain, const FSussContext& Context) const
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
