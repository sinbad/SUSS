
#include "SussAction.h"

#include "SussBrainComponent.h"

void USussAction::PerformAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TSubclassOf<USussAction> PrevActionClass)
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

UWorld* USussAction::GetWorld() const
{
	if (IsValid(Brain))
	{
		return Brain->GetWorld();
	}

	/// This is to allow this to function in the BP event graph and allow access to Delay() etc
	return Cast<UWorld>(GetOuter());
}

void USussAction::ActionCompleted()
{
	InternalOnActionCompleted.ExecuteIfBound(this);
}

void USussAction::DebugLocations_Implementation(TArray<FVector>& OutLocations, bool bIncludeDetails) const
{
	// Subclasses should implement
}
