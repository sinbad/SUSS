#include "SussTestSwapActions.h"

const FName USussTestSwapActionA::TagName("Suss.Action.Test.Swap.A");
const FName USussTestSwapActionB::TagName("Suss.Action.Test.Swap.B");
const FName USussTestSwapInputProvider::TagName("Suss.Input.Test.Swap");

TMap<FName, FSussTestSwapActionLog> USussTestSwapAction::Logs;
TFunction<void()> USussTestSwapAction::PerformHook;
TFunction<void()> USussTestSwapAction::CancelHook;
float USussTestSwapInputProvider::Value = 0;

void USussTestSwapAction::PerformAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TSubclassOf<USussAction> PreviousActionClass)
{
	auto& L = Log(GetClass());
	++L.NumPerformed;
	L.LastParams = Params;
	L.Latest = this;
	if (PerformHook)
	{
		TFunction<void()> Hook = MoveTemp(PerformHook);
		PerformHook = nullptr;
		Hook();
	}
}

void USussTestSwapAction::ContinueAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params)
{
	auto& L = Log(GetClass());
	++L.NumContinued;
	L.LastParams = Params;
}

void USussTestSwapAction::CancelAction_Implementation(TSubclassOf<USussAction> InterruptedByActionClass)
{
	++Log(GetClass()).NumCancelled;
	if (CancelHook)
	{
		TFunction<void()> Hook = MoveTemp(CancelHook);
		CancelHook = nullptr;
		Hook();
	}
}
