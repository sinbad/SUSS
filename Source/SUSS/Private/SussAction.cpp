
#include "SussAction.h"

#include "SussBrainComponent.h"

void USussAction::PerformAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TSubclassOf<USussAction> PrevActionClass)
{
	// Subclasses must implement
}

void USussAction::ContinueAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params)
{
	// Optional for subclasses to use this
}

void USussAction::CancelAction_Implementation(TSubclassOf<USussAction> InterruptedByActionClass)
{
	// Subclasses must implement
}

bool USussAction::CanBeInterrupted_Implementation() const
{
	return bAllowInterruptions;
}

void USussAction::Reset_Implementation()
{
	// Subclasses should implement
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
	// Allow BP to do something
	OnActionCompleted();
	InternalOnActionCompleted.ExecuteIfBound(this);
}

void USussAction::SetTemporaryActionScoreAdjustment(float Value, float CooldownTime)
{
	if (IsValid(Brain))
	{
		Brain->SetTemporaryActionScoreAdjustment(BrainActionIndex, Value, CooldownTime);
	}
}

void USussAction::AddTemporaryScoreAdjustment(float Value, float CooldownTime)
{
	if (IsValid(Brain))
	{
		Brain->AddTemporaryActionScoreAdjustment(BrainActionIndex, Value, CooldownTime);
	}
}

void USussAction::ResetTemporaryScoreAdjustment()
{
	if (IsValid(Brain))
	{
		Brain->ResetTemporaryActionScoreAdjustment(BrainActionIndex);
	}
}

void USussAction::DebugLocations_Implementation(TArray<FVector>& OutLocations, bool bIncludeDetails) const
{
	// Subclasses should implement
}
