#include "SussBrainComponent.h"
#include "SussGameSubsystem.h"
#include "SussTestSwapActions.h"
#include "SussTestWorldFixture.h"
#if WITH_AUTOMATION_TESTS

UE_DISABLE_OPTIMIZATION

/**
 * Changing a brain's config at runtime (SetBrainConfig / SetBrainConfigFromAsset) rebuilds and re-sorts its action list.
 * These tests pin down that the things keyed by position in that list - the action history, the running action's
 * index, and the running USussAction's own BrainActionIndex - move with their actions rather than staying put.
 *
 * Fixture: action A (priority 100, always scores 1) lives in set S; action B (priority 50, so it sorts in FRONT of A,
 * scoring the test input, which starts at 0) lives in set T. Each def carries a "Def" action param (A=1, B=2) so the
 * test can see which def the brain handed the running action. Neither action ever completes by itself.
 */
#if ENGINE_MAJOR_VERSION==5&&ENGINE_MINOR_VERSION>=5
BEGIN_DEFINE_SPEC(FSussBrainConfigSwapSpec,
				  "SUSS: Test  Config Swap",
				  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);
#else
BEGIN_DEFINE_SPEC(FSussBrainConfigSwapSpec,
				  "SUSS: Test  Config Swap",
				  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);
#endif

	TUniquePtr<FSussTestWorldFixture> WorldFixture;
	AActor* Self = nullptr;
	FGameplayTag TagA;
	FGameplayTag TagB;
	FGameplayTag TagInput;
	FSussActionDef DefA;
	FSussActionDef DefB;
	USussActionSetAsset* SetS = nullptr;
	USussActionSetAsset* SetT = nullptr;
	TArray<USussActionSetAsset*> RootedSets;

	static FName Marker() { return FName(TEXT("Def")); }

	USussActionSetAsset* MakeActionSet(const TArray<FSussActionDef>& Defs)
	{
		auto Set = NewObject<USussActionSetAsset>(GetTransientPackage());
		Set->AddToRoot();
		RootedSets.Add(Set);
		Set->GetActions() = Defs;
		return Set;
	}

	USussBrainComponent* MakeBrain(const FSussBrainConfig& Config)
	{
		auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));
		Brain->SetBrainConfig(Config);
		return Brain;
	}

	USussBrainComponent* MakeBrainWithSets(const TArray<USussActionSetAsset*>& Sets)
	{
		FSussBrainConfig Config;
		Config.ActionSets = Sets;
		return MakeBrain(Config);
	}

	/// Replace the brain's action sets, keeping the rest of its config
	static void SwapSets(USussBrainComponent* Brain, const TArray<USussActionSetAsset*>& Sets)
	{
		FSussBrainConfig Config = Brain->GetBrainConfig();
		Config.ActionSets = Sets;
		Brain->SetBrainConfig(Config);
	}

	static int32 IndexOfTag(const USussBrainComponent* Brain, const FGameplayTag& Tag)
	{
		return Brain->CombinedActionsByPriority.IndexOfByPredicate([&Tag](const FSussActionDef& Def) { return Def.ActionTag == Tag; });
	}

	/// The tag of the def the brain currently believes it is running, through its own index
	static FGameplayTag RunningTag(const USussBrainComponent* Brain)
	{
		const int Index = Brain->CurrentActionResult.ActionDefIndex;
		return Brain->CombinedActionsByPriority.IsValidIndex(Index) ? Brain->CombinedActionsByPriority[Index].ActionTag : FGameplayTag();
	}

	bool TestTag(const FString& What, const FGameplayTag& Actual, const FGameplayTag& Expected)
	{
		return TestEqual(What, Actual.ToString(), Expected.ToString());
	}

	static float LastParamValue(const UClass* ActionClass)
	{
		const FSussParameter* P = USussTestSwapAction::Log(ActionClass).LastParams.Find(Marker());
		return P ? P->FloatValue : -1.f;
	}

END_DEFINE_SPEC(FSussBrainConfigSwapSpec);


void FSussBrainConfigSwapSpec::Define()
{
	BeforeEach([this]()
	{
		WorldFixture = MakeUnique<FSussTestWorldFixture>();
		USussTestSwapAction::ResetAll();
		USussTestSwapInputProvider::Value = 0;

		TagA = FSussTestQueryTagHolder::Instance.GetTag(USussTestSwapActionA::TagName);
		TagB = FSussTestQueryTagHolder::Instance.GetTag(USussTestSwapActionB::TagName);
		TagInput = FSussTestQueryTagHolder::Instance.GetTag(USussTestSwapInputProvider::TagName);
		USussTestSwapAction::ConfigureDefaults(USussTestSwapActionA::StaticClass(), TagA, true);
		USussTestSwapAction::ConfigureDefaults(USussTestSwapActionB::StaticClass(), TagB, true);

		auto SUSS = GetSUSS(WorldFixture->GetWorld());
		SUSS->RegisterActionClass(USussTestSwapActionA::StaticClass());
		SUSS->RegisterActionClass(USussTestSwapActionB::StaticClass());
		SUSS->RegisterInputProviderClass(USussTestSwapInputProvider::StaticClass());

		DefA = FSussActionDef();
		DefA.ActionTag = TagA;
		DefA.Priority = 100;
		DefA.Inertia = 0;
		DefA.ActionParams.Add(Marker(), FSussParameter(1.0f));

		DefB = FSussActionDef();
		DefB.ActionTag = TagB;
		DefB.Priority = 50;
		DefB.Inertia = 0;
		DefB.ActionParams.Add(Marker(), FSussParameter(2.0f));
		FSussConsideration OnInput;
		OnInput.InputTag = TagInput;
		DefB.Considerations.Add(OnInput);

		SetS = MakeActionSet({ DefA });
		SetT = MakeActionSet({ DefB });
		Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
	});
	AfterEach([this]()
	{
		USussTestSwapAction::ResetAll();
		WorldFixture.Reset();
		for (auto Set : RootedSets)
		{
			Set->RemoveFromRoot();
		}
		RootedSets.Empty();
		FSussTestQueryTagHolder::Instance.UnregisterTags();
	});

	Describe("FSussBrainConfigSwapSpec", [this]()
	{
		It("A re-sorted running action is still read through its own def", [this]()
		{
			// A from S runs at index 0; adding T sorts B in front of it, so A moves to index 1
			auto Brain = MakeBrainWithSets({ SetS });
			Brain->Update();
			TestEqual("A running at index 0", Brain->CurrentActionResult.ActionDefIndex, 0);

			SwapSets(Brain, { SetS, SetT });
			TestFalse("Nothing pending: A survived the swap", Brain->IsConfigChangePending());
			TestEqual("Two actions", Brain->CombinedActionsByPriority.Num(), 2);
			TestTag("B sorted first", Brain->CombinedActionsByPriority[0].ActionTag, TagB);
			TestEqual("Running index moved with A", Brain->CurrentActionResult.ActionDefIndex, 1);
			TestTag("Brain reads its running action as A", RunningTag(Brain), TagA);

			// B still scores 0, so A keeps winning: it must be CONTINUED with A's params, not cancelled and re-performed
			Brain->Update();
			TestTag("Still A", RunningTag(Brain), TagA);
			TestEqual("Not cancelled", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestEqual("Performed once", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumPerformed, 1);
			TestEqual("Continued once", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumContinued, 1);
			TestEqual("Continued with A's own params", LastParamValue(USussTestSwapActionA::StaticClass()), 1.0f);
		});

		It("A re-sorted running action's own score adjustments land on its own history entry", [this]()
		{
			// The running USussAction keeps its own copy of the index (BrainActionIndex), which its score adjustment
			// calls use; it has to move too
			auto Brain = MakeBrainWithSets({ SetS });
			Brain->Update();
			USussAction* Live = USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).Latest;
			if (!TestNotNull("A is running", Live))
			{
				return;
			}

			SwapSets(Brain, { SetS, SetT });
			Live->AddTemporaryScoreAdjustment(3.0f, 0);
			const int32 IndexA = IndexOfTag(Brain, TagA);
			TestEqual("A is at index 1", IndexA, 1);
			TestEqual("Nudge landed on A's entry", Brain->ActionHistory[1].TempScoreAdjust, 3.0f);
			TestEqual("B came in clean", Brain->ActionHistory[0].TempScoreAdjust, 0.0f);

			Brain->Update();
			Live->AddTemporaryScoreAdjustment(1.0f, 0);
			TestEqual("Still A's entry after the next update", Brain->ActionHistory[1].TempScoreAdjust, 4.0f);
			TestEqual("B still clean", Brain->ActionHistory[0].TempScoreAdjust, 0.0f);
		});

		It("A swap that removes the running action waits, then stops it at the next update", [this]()
		{
			// B is the brain's OWN def here, A comes from S: list is [B, A], A runs at index 1. Dropping S leaves a
			// one-entry list, which the old index 1 would read past the end of.
			FSussBrainConfig Config;
			Config.ActionSets = { SetS };
			Config.ActionDefs.Add(DefB);
			auto Brain = MakeBrain(Config);
			Brain->Update();
			TestEqual("A running at index 1", Brain->CurrentActionResult.ActionDefIndex, 1);
			TestFalse("No update queued", Brain->NeedsUpdate());

			SwapSets(Brain, {});
			// Nothing moves yet: the old list, history and index stay consistent, and an update is queued to apply it
			TestTrue("Change is pending", Brain->IsConfigChangePending());
			TestEqual("Old list still whole", Brain->CombinedActionsByPriority.Num(), 2);
			TestEqual("Index untouched", Brain->CurrentActionResult.ActionDefIndex, 1);
			TestTag("Brain still reads A", RunningTag(Brain), TagA);
			TestEqual("Not cancelled yet", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestTrue("Update queued to apply the change", Brain->NeedsUpdate());

			Brain->Update();
			// Stopped through the ordinary path, then the new list committed
			TestEqual("Cancelled once", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 1);
			TestFalse("Nothing in progress", Brain->IsActionInProgress());
			TestEqual("No current action", Brain->CurrentActionResult.ActionDefIndex, (int)INDEX_NONE);
			TestFalse("Change applied", Brain->IsConfigChangePending());
			TestEqual("New list has one entry", Brain->CombinedActionsByPriority.Num(), 1);
			TestTag("Which is B", Brain->CombinedActionsByPriority[0].ActionTag, TagB);
			TestEqual("History resized with it", Brain->ActionHistory.Num(), 1);
			TestTrue("B's history untouched (never started)", Brain->ActionHistory[0].LastStartTime == -UE_DOUBLE_BIG_NUMBER);

			// And the update after runs cleanly on the new list: B now scores, and is chosen with its own params
			USussTestSwapInputProvider::Value = 1;
			Brain->Update();
			TestTag("B chosen", RunningTag(Brain), TagB);
			TestEqual("B performed", USussTestSwapAction::Log(USussTestSwapActionB::StaticClass()).NumPerformed, 1);
			TestEqual("With B's own params", LastParamValue(USussTestSwapActionB::StaticClass()), 2.0f);
		});

		It("A set listed twice keeps the running copy when one listing is dropped", [this]()
		{
			// Two copies of A with the same origin. The SECOND copy runs; a swap to S listed once must keep that one
			// (it isn't stopped, and it keeps its own history), not hand the surviving entry to its twin.
			auto Brain = MakeBrainWithSets({ SetS, SetS });
			TestEqual("Two copies", Brain->CombinedActionsByPriority.Num(), 2);
			Brain->SetTemporaryActionScoreAdjustment(1, 1.0f, 0);   // lifts copy 1 above copy 0
			Brain->Update();
			TestEqual("Second copy running", Brain->CurrentActionResult.ActionDefIndex, 1);

			SwapSets(Brain, { SetS });
			TestFalse("Nothing pending", Brain->IsConfigChangePending());
			TestEqual("One entry", Brain->CombinedActionsByPriority.Num(), 1);
			TestEqual("Not cancelled", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestTrue("Still in progress", Brain->IsActionInProgress());
			TestEqual("Running copy moved to index 0", Brain->CurrentActionResult.ActionDefIndex, 0);
			TestEqual("Surviving entry carries the running copy's history", Brain->ActionHistory[0].TempScoreAdjust, 1.0f);

			Brain->Update();
			TestEqual("Not cancelled", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestEqual("Continued", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumContinued, 1);
		});

		It("A swap made inside the running action's PerformAction waits for the next update", [this]()
		{
			// ChooseAction holds its chosen def by reference across PerformAction, so the list must not move under it
			// even though A survives the swap. Start with [A (S), B (U, priority 150 so it sorts last)]; the swap brings
			// in T, whose B sorts in front of A - same size list, so an in-place rebuild would change what the held
			// reference points at.
			FSussActionDef DefBLast = DefB;
			DefBLast.Priority = 150;
			USussActionSetAsset* SetU = MakeActionSet({ DefBLast });
			auto Brain = MakeBrainWithSets({ SetS, SetU });
			TestEqual("Two actions", Brain->CombinedActionsByPriority.Num(), 2);
			TestTag("A first", Brain->CombinedActionsByPriority[0].ActionTag, TagA);

			USussTestSwapAction::PerformHook = [Brain, this]() { SwapSets(Brain, { SetS, SetT }); };
			Brain->Update();
			TestFalse("Hook ran", (bool)USussTestSwapAction::PerformHook);
			TestEqual("A performed", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumPerformed, 1);
			// The list did not move; the new config is waiting, and an update is queued for it
			TestTrue("Change is pending", Brain->IsConfigChangePending());
			TestTag("A still first", Brain->CombinedActionsByPriority[0].ActionTag, TagA);
			TestEqual("A's start recorded on A's own entry", Brain->CurrentActionResult.ActionDefIndex, 0);
			TestTrue("A's history shows the start", Brain->ActionHistory[0].LastStartTime > -UE_DOUBLE_BIG_NUMBER);
			TestEqual("Config already holds T", Brain->GetBrainConfig().ActionSets[1], SetT);
			TestTrue("Update queued", Brain->NeedsUpdate());

			Brain->Update();
			TestFalse("Change applied", Brain->IsConfigChangePending());
			TestTag("B now first", Brain->CombinedActionsByPriority[0].ActionTag, TagB);
			TestEqual("A re-indexed", Brain->CurrentActionResult.ActionDefIndex, 1);
			TestTag("Brain reads A", RunningTag(Brain), TagA);
			TestTrue("A's history moved with it", Brain->ActionHistory[1].LastStartTime > -UE_DOUBLE_BIG_NUMBER);
			TestEqual("Not cancelled", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestEqual("Continued", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumContinued, 1);
		});

		It("A swap made from the removed action's CancelAction applies in the same update", [this]()
		{
			// The pending swap removes A; at the next update the brain stops A, whose CancelAction puts S back (plus T).
			// That inner change is deferred (we're inside Update) and applied by the second rebuild of the same update.
			auto Brain = MakeBrainWithSets({ SetS });
			Brain->Update();

			USussTestSwapAction::CancelHook = [Brain, this]() { SwapSets(Brain, { SetS, SetT }); };
			SwapSets(Brain, { SetT });
			TestTrue("Change is pending", Brain->IsConfigChangePending());
			TestTag("Brain still reads A", RunningTag(Brain), TagA);

			Brain->Update();
			TestEqual("Cancelled exactly once (no recursion)", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 1);
			TestFalse("Hook ran", (bool)USussTestSwapAction::CancelHook);
			TestFalse("Change applied", Brain->IsConfigChangePending());
			TestEqual("Config has both sets", Brain->GetBrainConfig().ActionSets.Num(), 2);
			TestEqual("List built from the hook's config", Brain->CombinedActionsByPriority.Num(), 2);
			TestTag("B first", Brain->CombinedActionsByPriority[0].ActionTag, TagB);
			// A is back in the config, so this same update chose it afresh (B still scores 0)
			TestTag("A chosen again", RunningTag(Brain), TagA);
			TestEqual("A performed twice", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumPerformed, 2);
		});

		It("A removed action that cannot be interrupted runs on, and the swap applies after it ends", [this]()
		{
			USussTestSwapAction::ConfigureDefaults(USussTestSwapActionA::StaticClass(), TagA, false);
			auto Brain = MakeBrainWithSets({ SetS });
			Brain->Update();
			USussAction* Live = USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).Latest;
			if (!TestNotNull("A is running", Live))
			{
				return;
			}
			TestFalse("A cannot be interrupted", Live->CanBeInterrupted());

			SwapSets(Brain, { SetT });
			TestTrue("Change is pending", Brain->IsConfigChangePending());

			Brain->Update();
			TestEqual("Not cancelled", USussTestSwapAction::Log(USussTestSwapActionA::StaticClass()).NumCancelled, 0);
			TestTrue("Still in progress", Brain->IsActionInProgress());
			TestTag("Brain still reads A", RunningTag(Brain), TagA);
			TestTrue("Change still pending", Brain->IsConfigChangePending());
			TestEqual("Old list still in use", Brain->CombinedActionsByPriority.Num(), 1);
			TestTag("Which is A", Brain->CombinedActionsByPriority[0].ActionTag, TagA);
			TestEqual("New config waiting", Brain->GetBrainConfig().ActionSets[0], SetT);

			Live->ActionCompleted();
			TestTrue("Completion queued the update that applies the swap", Brain->NeedsUpdate());
			USussTestSwapInputProvider::Value = 1;
			Brain->Update();
			TestFalse("Change applied", Brain->IsConfigChangePending());
			TestEqual("New list", Brain->CombinedActionsByPriority.Num(), 1);
			TestTag("Which is B", Brain->CombinedActionsByPriority[0].ActionTag, TagB);
			TestTag("B chosen", RunningTag(Brain), TagB);
		});

		It("IsConfigChangePending reports whether the action list has caught up with the config", [this]()
		{
			auto Brain = MakeBrainWithSets({ SetS });
			TestFalse("False after the first build", Brain->IsConfigChangePending());
			Brain->Update();

			SwapSets(Brain, { SetS, SetT });
			TestFalse("False after a swap the running action survives (applied at once)", Brain->IsConfigChangePending());

			SwapSets(Brain, { SetT });
			TestTrue("True after a swap that removes the running action", Brain->IsConfigChangePending());
			TestEqual("Config is already the new one", Brain->GetBrainConfig().ActionSets.Num(), 1);
			TestEqual("But the list is still the old one", Brain->CombinedActionsByPriority.Num(), 2);

			Brain->Update();
			TestFalse("False once the update has applied it", Brain->IsConfigChangePending());
			TestEqual("List caught up", Brain->CombinedActionsByPriority.Num(), 1);
		});
	});
}

UE_ENABLE_OPTIMIZATION

#endif
