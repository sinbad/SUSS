#pragma once

#include "CoreMinimal.h"
#include "SussAction.h"
#include "SussInputProvider.h"
#include "SussTestQueryProviders.h"
#include "SussTestSwapActions.generated.h"

/// What a test action class has seen so far in the current test
struct FSussTestSwapActionLog
{
	int NumPerformed = 0;
	int NumContinued = 0;
	int NumCancelled = 0;
	/// The action params the brain passed on the last PerformAction / ContinueAction
	TMap<FName, FSussParameter> LastParams;
	/// The most recently performed instance
	USussAction* Latest = nullptr;
};

/**
 * A test action which never completes by itself and records what the brain did to it.
 * Tags are registered at runtime (see FSussTestQueryTagHolder), so the CDO's tag is set per test via ConfigureDefaults
 * rather than in the constructor (StaticClass is too early for temp tags).
 */
UCLASS(Abstract)
class USussTestSwapAction : public USussAction
{
	GENERATED_BODY()
public:
	/// Per-class logs, keyed by class name; reset between tests
	static TMap<FName, FSussTestSwapActionLog> Logs;
	/// Runs once from inside the next PerformAction, then clears itself
	static TFunction<void()> PerformHook;
	/// Runs once from inside the next CancelAction, then clears itself
	static TFunction<void()> CancelHook;

	static FSussTestSwapActionLog& Log(const UClass* Class) { return Logs.FindOrAdd(Class->GetFName()); }
	static void ResetAll()
	{
		Logs.Empty();
		PerformHook = nullptr;
		CancelHook = nullptr;
	}

	/// Set the tag (and interruptibility) on the class default object; RegisterActionClass reads the tag from there
	static void ConfigureDefaults(UClass* Class, const FGameplayTag& Tag, bool bInterruptible)
	{
		if (auto CDO = GetMutableDefault<USussTestSwapAction>(Class))
		{
			CDO->ActionTag = Tag;
			CDO->bAllowInterruptions = bInterruptible;
		}
	}

	/// An instance of a native class is not initialised from a CDO edited at runtime, so take the configured values
	/// from it here (Init calls Reset on every reserve, so a pooled instance picks them up too)
	virtual void Reset_Implementation() override
	{
		const USussTestSwapAction* CDO = GetDefault<USussTestSwapAction>(GetClass());
		ActionTag = CDO->ActionTag;
		bAllowInterruptions = CDO->bAllowInterruptions;
	}

	virtual void PerformAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params, TSubclassOf<USussAction> PreviousActionClass) override;
	virtual void ContinueAction_Implementation(const FSussContext& Context, const TMap<FName, FSussParameter>& Params) override;
	virtual void CancelAction_Implementation(TSubclassOf<USussAction> InterruptedByActionClass) override;
};

UCLASS()
class USussTestSwapActionA : public USussTestSwapAction
{
	GENERATED_BODY()
public:
	static const FName TagName;
};

UCLASS()
class USussTestSwapActionB : public USussTestSwapAction
{
	GENERATED_BODY()
public:
	static const FName TagName;
};

/// An input whose value the test sets directly, so that an action can be made to score 0 or 1 at will
UCLASS()
class USussTestSwapInputProvider : public USussInputProvider
{
	GENERATED_BODY()
public:
	static const FName TagName;
	static float Value;

	virtual FGameplayTag GetInputTag() const override
	{
		return FSussTestQueryTagHolder::Instance.GetTag(TagName);
	}

	virtual float Evaluate_Implementation(const USussBrainComponent* Brain, const FSussContext& Context, const TMap<FName, FSussParameter>& Parameters) const override
	{
		return Value;
	}
};
