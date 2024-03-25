
#include "SussAction.h"

void USussAction::PerformAction_Implementation(const FSussContext& Context, TSubclassOf<USussAction> PrevActionClass)
{
	// Subclasses must implement
}

void USussAction::CancelAction_Implementation(TSubclassOf<USussAction> InterruptedByActionClass)
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

void USussAction::DebugLocations_Implementation(TArray<FVector>& OutLocations) const
{
	// Subclasses should implement
}
