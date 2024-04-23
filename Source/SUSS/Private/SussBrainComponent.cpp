#include "SussBrainComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "SussAction.h"
#include "SussBrainConfigAsset.h"
#include "SussCommon.h"
#include "SussGameSubsystem.h"
#include "SussPoolSubsystem.h"
#include "SussSettings.h"
#include "SussUtility.h"
#include "SussWorldSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"


// Sets default values for this component's properties
USussBrainComponent::USussBrainComponent(): bQueuedForUpdate(false),
                                            bWasPreventedFromUpdating(false),
                                            BrainConfigAsset(nullptr),
                                            DistanceCategory(ESussDistanceCategory::OutOfRange),
                                            CurrentUpdateInterval(0),
                                            CurrentActionResult(),
                                            PerceptionComp(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;

	// Disable ticking by default
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void USussBrainComponent::SetBrainConfig(const FSussBrainConfig& NewConfig)
{
	// Note that we don't do anything with the current action until we need to change our minds
	BrainConfig = NewConfig;
	BrainConfigChanged();
}

void USussBrainComponent::SetBrainConfigFromAsset(USussBrainConfigAsset* Asset)
{
	BrainConfig = Asset->BrainConfig;
	BrainConfigChanged();
}

void USussBrainComponent::BrainConfigChanged()
{
	if (GetOwner()->HasAuthority())
	{
		InitActions();
	}
}



// Called when the game starts
void USussBrainComponent::BeginPlay()
{
	Super::BeginPlay();

	if (auto AIController = GetAIController())
	{
		PerceptionComp = GetOwner()->FindComponentByClass<UAIPerceptionComponent>();
	}


	if (auto Settings = GetDefault<USussSettings>())
	{
		if (PerceptionComp && Settings->BrainUpdateOnPerceptionChanges)
		{
			PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &USussBrainComponent::OnPerceptionUpdated);
		}
	}
}


void USussBrainComponent::StartLogic()
{
	Super::StartLogic();

	if (GetOwner()->HasAuthority())
	{
		UpdateDistanceCategory();

		if (IsValid(BrainConfigAsset))
		{
			if (BrainConfig.ActionDefs.Num() || BrainConfig.ActionSets.Num())
			{
				UE_LOG(LogSuss, Warning, TEXT("SUSS embedded BrainConfig is being overwritten by asset link on BeginPlay"))
			}
			SetBrainConfigFromAsset(BrainConfigAsset);
		}
		else
		{
			BrainConfigChanged();
		}

		if (BrainConfig.PreventBrainUpdateIfAnyTags.Num() > 0)
		{
			if (auto Pawn = GetPawn())
			{
				// Listen on gameplay tag changes
				if (auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
				{
					for (auto Tag : BrainConfig.PreventBrainUpdateIfAnyTags)
					{
						TagDelegates.Add(Tag, ASC->RegisterGameplayTagEvent(Tag).AddUObject(this, &USussBrainComponent::OnGameplayTagEvent));
					}
				}
			}
		}
	}
	
}

float USussBrainComponent::GetDistanceToAnyPlayer() const
{
	auto Pawn = GetPawn();
	if (IsValid(Pawn))
	{
		const auto World = GetWorld();
		const FVector OurPos = Pawn->GetActorLocation();
		float MinSqDistance = std::numeric_limits<float>::max();

		for (int i = 0; i < UGameplayStatics::GetNumPlayerControllers(World); ++i)
		{
			auto PlayerPawn = UGameplayStatics::GetPlayerPawn(World, i);
			if (IsValid(PlayerPawn))
			{
				MinSqDistance = FMath::Min(MinSqDistance, FVector::DistSquared(OurPos, PlayerPawn->GetActorLocation()));
			}
		}

		return FMath::Sqrt(MinSqDistance);
	}

	return std::numeric_limits<float>::max();
}

void USussBrainComponent::UpdateDistanceCategory()
{
	const float Dist = GetDistanceToAnyPlayer();
	float NewInterval = 1.0f;
	
	if (const auto Settings = GetDefault<USussSettings>())
	{
		if (Dist <= Settings->NearAgentSettings.MaxDistance)
		{
			DistanceCategory = ESussDistanceCategory::Near;
			NewInterval = Settings->NearAgentSettings.BrainUpdateRequestIntervalSeconds;
		}
		else if (Dist <= Settings->MidRangeAgentSettings.MaxDistance)
		{
			DistanceCategory = ESussDistanceCategory::MidRange;
			NewInterval = Settings->MidRangeAgentSettings.BrainUpdateRequestIntervalSeconds;
		}
		else if (Dist <= Settings->FarAgentSettings.MaxDistance)
		{
			DistanceCategory = ESussDistanceCategory::Far;
			NewInterval = Settings->FarAgentSettings.BrainUpdateRequestIntervalSeconds;
		}
		else
		{
			DistanceCategory = ESussDistanceCategory::OutOfRange;
			NewInterval = Settings->OutOfBoundsDistanceCheckInterval;
		}
	}

	auto& TM = GetWorld()->GetTimerManager();

	if (!UpdateRequestTimer.IsValid() || NewInterval != CurrentUpdateInterval)
	{
		// Randomise the time that brains start their update to spread them out
		float Delay = FMath::RandRange(0.0f, NewInterval);
		TM.SetTimer(UpdateRequestTimer, this, &USussBrainComponent::TimerCallback, NewInterval, true, Delay);
		CurrentUpdateInterval = NewInterval;
	}

	// Just in case this somehow gets called while agent is paused
	if (IsPaused())
	{
		TM.PauseTimer(UpdateRequestTimer);
	}
}

void USussBrainComponent::StopLogic(const FString& Reason)
{
	Super::StopLogic(Reason);

	StopCurrentAction();
	if (UpdateRequestTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateRequestTimer);
	}

	if (TagDelegates.Num() > 0)
	{
		if (const auto Pawn = GetPawn())
		{
			// Listen on gameplay tag changes
			if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
			{
				for (const auto Pair : TagDelegates)
				{
					ASC->UnregisterGameplayTagEvent(Pair.Value, Pair.Key);
				}
			}
		}

		TagDelegates.Empty();
	}

}

void USussBrainComponent::RestartLogic()
{
	Super::RestartLogic();

	StopCurrentAction();
	UpdateDistanceCategory();
}

void USussBrainComponent::PauseLogic(const FString& Reason)
{
	Super::PauseLogic(Reason);

	if (UpdateRequestTimer.IsValid())
	{
		GetWorld()->GetTimerManager().PauseTimer(UpdateRequestTimer);
	}
}

EAILogicResuming::Type USussBrainComponent::ResumeLogic(const FString& Reason)
{
	auto Ret = Super::ResumeLogic(Reason);
	if (Ret != EAILogicResuming::RestartedInstead)
	{
		// restarted calls RestartLogic
		if (UpdateRequestTimer.IsValid())
		{
			GetWorld()->GetTimerManager().UnPauseTimer(UpdateRequestTimer);
		}
	}
	return Ret;
}

void USussBrainComponent::InitActions()
{
	// Collate all the actions from referenced action sets, and actions only on this instance
	CombinedActionsByPriority.Empty();
	for (auto ActionSet : BrainConfig.ActionSets)
	{
		// Guard against bad config
		if (IsValid(ActionSet))
		{
			for (auto& Action : ActionSet->GetActions())
			{
				CombinedActionsByPriority.Add(Action);
			}
		}
	}
	for (auto& Action : BrainConfig.ActionDefs)
	{
		CombinedActionsByPriority.Add(Action);
	}

	// Sort by ascending priority
	CombinedActionsByPriority.Sort([](const FSussActionDef& A, const FSussActionDef& B)
	{
		return A.Priority < B.Priority;
	});
}


void USussBrainComponent::RequestUpdate()
{
	if (GetOwner()->HasAuthority())
	{
		QueueForUpdate();
	}
}

bool USussBrainComponent::IsUpdatePrevented() const
{
	if (BrainConfig.PreventBrainUpdateIfAnyTags.Num() > 0)
	{
		if (auto Pawn = GetPawn())
		{
			// Listen on gameplay tag changes
			if (auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
			{
				if (ASC->HasAnyMatchingGameplayTags(BrainConfig.PreventBrainUpdateIfAnyTags))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void USussBrainComponent::QueueForUpdate()
{
	if (!bQueuedForUpdate)
	{
		if (IsUpdatePrevented())
		{
			bWasPreventedFromUpdating = true;
		}
		else
		{
			if (auto SS = GetSussWorldSubsystem(GetWorld()))
			{
				SS->QueueBrainUpdate(this);
				bQueuedForUpdate = true;
				bWasPreventedFromUpdating = false;
			}
		}
	}
}

void USussBrainComponent::OnGameplayTagEvent(const FGameplayTag InTag, int32 NewCount)
{
	// By nature this has to be one of the brain config's prevent update tags
	// We don't need to check > 0 because that's checked on update
	// We just need to check if we need to immediately update
	if (NewCount == 0 && bWasPreventedFromUpdating)
	{
		// This will check for the presence of any blocking tags again
		QueueForUpdate();
	}
}

void USussBrainComponent::TimerCallback()
{
	if (CurrentActionInertiaCooldown > 0)
	{
		CurrentActionInertiaCooldown = FMath::Max(CurrentActionInertiaCooldown - CurrentUpdateInterval, 0);
	}

	UpdateDistanceCategory();

	// We still get timer callbacks for being out of range, we simply check the distance
	if (DistanceCategory != ESussDistanceCategory::OutOfRange)
	{
		QueueForUpdate();
	}
}

void USussBrainComponent::ChooseActionFromCandidates()
{
	if (CandidateActions.IsEmpty())
	{
		return;
	}

	CandidateActions.Sort([](const FSussActionScoringResult& L, const FSussActionScoringResult& R)
	{
		// sort from highest to lowest
		return L.Score > R.Score;
	});

	if (BrainConfig.ActionChoiceMethod == ESussActionChoiceMethod::HighestScoring)
	{
#if ENABLE_VISUAL_LOG
		UE_VLOG(this, LogSuss, Log, TEXT("Choice method: Highest Scoring"));
#endif
		ChooseAction(CandidateActions[0]);
	}
	else
	{
		// Weighted random of some kind
		float TotalScores = 0;
		int ChoiceCount = 0;
		const float BestScore = CandidateActions[0].Score;
		const float ScoreLimit = BrainConfig.ActionChoiceMethod == ESussActionChoiceMethod::WeightedRandomTopNPercent ?
			BestScore * 100.0f / (float)BrainConfig.ActionChoiceTopN : 0;
		for (int i = 0; i < CandidateActions.Num(); ++i, ++ChoiceCount)
		{
			if (BrainConfig.ActionChoiceMethod == ESussActionChoiceMethod::WeightedRandomTopN)
			{
				if (i == BrainConfig.ActionChoiceTopN)
				{
					break;
				}
			}
			else if (BrainConfig.ActionChoiceMethod == ESussActionChoiceMethod::WeightedRandomTopNPercent)
			{
				if (CandidateActions[i].Score < ScoreLimit)
				{
					break;
				}
			}

			TotalScores += CandidateActions[i].Score;
		}

		const float Rand = FMath::RandRange(0.0f, TotalScores);
		float ScoreAccum = 0;
		for (int i = 0; i < ChoiceCount; ++i)
		{
			ScoreAccum += CandidateActions[i].Score;

			if (Rand < ScoreAccum)
			{
#if ENABLE_VISUAL_LOG
				UE_VLOG(this, LogSuss, Log,
				        TEXT("Choice method: %s (%d) [%4.2f/%4.2f]"),
				        *StaticEnum<ESussActionChoiceMethod>()->GetDisplayNameTextByValue((int64)BrainConfig.
					        ActionChoiceMethod).ToString(),
					    BrainConfig.ActionChoiceTopN,
				        Rand,
				        TotalScores);
#endif
				ChooseAction(CandidateActions[i]);
				break;
			}
		}
	}
}

void USussBrainComponent::StopCurrentAction()
{
	CancelCurrentAction(nullptr);
}
void USussBrainComponent::CancelCurrentAction(TSubclassOf<USussAction> Interrupter)
{
	// Cancel previous action
	if (IsValid(CurrentActionInstance))
	{
		CurrentActionInstance->InternalOnActionCompleted.Unbind();
		CurrentActionInstance->CancelAction(Interrupter);
		CurrentActionInstance = nullptr;
		CurrentActionResult.ActionDefIndex = -1;
	}
}

bool USussBrainComponent::IsActionInProgress()
{
	return CurrentActionInstance != nullptr;
}

void USussBrainComponent::ChooseAction(const FSussActionScoringResult& ActionResult)
{
	checkf(ActionResult.ActionDefIndex >= 0, TEXT("No supplied action def"));

	const FSussActionDef& Def = CombinedActionsByPriority[ActionResult.ActionDefIndex];
	if (IsValid(CurrentActionInstance) &&
		ActionResult.ActionDefIndex == CurrentActionResult.ActionDefIndex &&
		ActionResult.Context == CurrentActionResult.Context)
	{
		// We're already running it, so just continue
#if ENABLE_VISUAL_LOG
		UE_VLOG(this, LogSuss, Log, TEXT("No Action Change, continue: %s %s"), Def.Description.IsEmpty() ? *Def.ActionTag.ToString() : *Def.Description, *ActionResult.Context.ToString());
#endif
		CurrentActionInstance->ContinueAction(ActionResult.Context, Def.ActionParams);
		return;
	}

	auto SUSS = GetSUSS(GetWorld());
	const TSubclassOf<USussAction> ActionClass = SUSS->GetActionClass(Def.ActionTag);

#if ENABLE_VISUAL_LOG
	UE_VLOG(this, LogSuss, Log, TEXT("Chose NEW action: %s %s"), Def.Description.IsEmpty() ? *Def.ActionTag.ToString() : *Def.Description, *ActionResult.Context.ToString());
#endif

	TSubclassOf<USussAction> PreviousActionClass = nullptr;
	if (IsValid(CurrentActionInstance))
	{
		PreviousActionClass = CurrentActionInstance->GetClass();
	}
	StopCurrentAction();
	CurrentActionResult = ActionResult;
	CurrentActionInertiaCooldown = Def.InertiaCooldown;
	ActionsTimeLastPerformed.Add(Def.ActionTag, GetWorld()->GetTimeSeconds());

	if (ActionClass)
	{
		// Note that to allow BP classes we need to construct using the default object
		CurrentActionInstance = GetSussPool(GetWorld())->ReserveAction(ActionClass, this, ActionClass->GetDefaultObject());
		CurrentActionInstance->Init(this, ActionResult.Context);
		CurrentActionInstance->InternalOnActionCompleted.BindUObject(this, &USussBrainComponent::OnActionCompleted);
		CurrentActionInstance->PerformAction(ActionResult.Context, Def.ActionParams, PreviousActionClass);
	}
	else
	{
		// No action class provided for this tag, do nothing
		CurrentActionInstance = nullptr;

		UE_LOG(LogSuss, Warning, TEXT("No action class for tag %s, so doing nothing"), *Def.ActionTag.ToString());
		
	}

	
}

void USussBrainComponent::OnActionCompleted(USussAction* SussAction)
{
	// Sometimes possible for actions to call us back late when we've already abandoned them, ignore that
	if (IsValid(CurrentActionInstance) && CurrentActionInstance == SussAction)
	{
		GetSussPool(GetWorld())->FreeAction(CurrentActionInstance);
		CurrentActionInstance = nullptr;
		CurrentActionResult.ActionDefIndex = -1;
		CurrentActionResult.Score = 0;
		CurrentActionInertiaCooldown = 0;
		// Immediately queue for update so no hesitation after completion
		QueueForUpdate();

		SussAction->InternalOnActionCompleted.Unbind();
	}

}

void USussBrainComponent::Update()
{
	bQueuedForUpdate = false;
	
	if (!GetOwner()->HasAuthority())
		return;

	if (CombinedActionsByPriority.IsEmpty())
		return;

	/// If we can't be interrupted, no need to check what else we could be doing
	if (IsValid(CurrentActionInstance) && !CurrentActionInstance->CanBeInterrupted())
		return;

#if ENABLE_VISUAL_LOG
	UE_VLOG(this, LogSuss, Log, TEXT("Brain Update"));
#endif

	auto SUSS = GetSUSS(GetWorld());
	auto Pool = GetSussPool(GetWorld());
	AActor* Self = GetSelf();

	const FSussActionDef* CurrentActionDef = IsActionInProgress() ? &CombinedActionsByPriority[CurrentActionResult.ActionDefIndex] : nullptr;
	
	if (CurrentActionDef && CurrentActionDef->Inertia > 0 && CurrentActionDef->InertiaCooldown > 0)
	{
		CurrentActionInertia = CurrentActionDef->Inertia * (CurrentActionInertiaCooldown / CurrentActionDef->InertiaCooldown);
	}
	else
	{
		CurrentActionInertia = 0;
	}
	
	int CurrentPriority = CombinedActionsByPriority[0].Priority;
	// Use reset not empty in order to keep memory stable
	CandidateActions.Reset();
	for (int i = 0; i < CombinedActionsByPriority.Num(); ++i)
	{
		const FSussActionDef& NextAction = CombinedActionsByPriority[i];
		// Priority grouping - use the best option from the highest priority group first
		if (CurrentPriority != NextAction.Priority)
		{
			// End of priority group
			if (!CandidateActions.IsEmpty())
			{
				// OK we pick from these & don't consider the others
				break;
			}

			// Otherwise we had no candidates in that group, carry on to the next one
			CurrentPriority = NextAction.Priority;
		}

		// Ignore zero-weighted actions
		if (NextAction.Weight < UE_KINDA_SMALL_NUMBER)
			continue;

		// Ignore bad config or globally disabled actions
		if (!NextAction.ActionTag.IsValid() || !USussUtility::IsActionEnabled(NextAction.ActionTag))
			continue;

		// Check required/blocking tags on self
		if (NextAction.RequiredTags.Num() > 0 && !USussUtility::ActorHasAllTags(GetOwner(), NextAction.RequiredTags))
			continue;
		if (NextAction.BlockingTags.Num() > 0 && USussUtility::ActorHasAnyTags(GetOwner(), NextAction.BlockingTags))
			continue;

		auto ArrayPool = GetSussPool(GetWorld());
		
		FSussScopeReservedArray ContextsScope = ArrayPool->ReserveArray<FSussContext>();
		TArray<FSussContext>& Contexts = *ContextsScope.Get<FSussContext>();
		GenerateContexts(Self, NextAction, Contexts);

#if ENABLE_VISUAL_LOG
		UE_VLOG(this, LogSuss, Log, TEXT("Action: %s  Priority: %d Weight: %4.2f Contexts: %d"),
			NextAction.Description.IsEmpty() ? *NextAction.ActionTag.ToString() : *NextAction.Description,
			NextAction.Priority,
			NextAction.Weight,
			Contexts.Num());
#endif
		
		// Evaluate this action for every applicable context
		for (const auto& Ctx : Contexts)
		{
#if ENABLE_VISUAL_LOG
			UE_VLOG(this, LogSuss, Log, TEXT(" - %s"), *Ctx.ToString());
#endif
			float Score = NextAction.Weight;
			for (auto& Consideration : NextAction.Considerations)
			{
				if (auto InputProvider = SUSS->GetInputProvider(Consideration.InputTag))
				{
					// Resolve parameters
					FSussScopeReservedMap ResolvedQueryParamsScope = Pool->ReserveMap<FName, FSussParameter>();
					TMap<FName, FSussParameter>& ResolvedParams = *ResolvedQueryParamsScope.Get<FName, FSussParameter>();
					ResolveParameters(Self, Consideration.Parameters, ResolvedParams);

					const float RawInputValue = InputProvider->Evaluate(this, Ctx, ResolvedParams);

					// Normalise to bookends and clamp
					const float NormalisedInput = FMath::Clamp(FMath::GetRangePct(
						                                           ResolveParameter(
							                                           Ctx,
							                                           Consideration.BookendMin).FloatValue,
						                                           ResolveParameter(
							                                           Ctx,
							                                           Consideration.BookendMax).FloatValue,
						                                           RawInputValue),
					                                           0.f,
					                                           1.f);

					// Transform through curve
					const float ConScore = Consideration.EvaluateCurve(NormalisedInput);

#if ENABLE_VISUAL_LOG
					UE_VLOG(this, LogSuss, Log, TEXT("  * Consideration: %s  Input: %4.2f  Normalised: %4.2f  Final: %4.2f"),
						Consideration.Description.IsEmpty() ? *Consideration.InputTag.ToString() : *Consideration.Description,
						RawInputValue, NormalisedInput, ConScore);
#endif

					// Accumulate with overall score
					Score *= ConScore;

					// Early-out if we've ended up at zero, nothing can change this now
					if (FMath::IsNearlyZero(Score))
					{
						break;
					}
					
				}
			}

			// Add inertia if this is the current action
			if (IsActionInProgress() && i == CurrentActionResult.ActionDefIndex)
			{
				Score += CurrentActionInertia;
#if ENABLE_VISUAL_LOG
				UE_VLOG(this, LogSuss, Log, TEXT("  * Current Action Inertia: %4.2f"), CurrentActionInertia);
#endif
			}

#if ENABLE_VISUAL_LOG
			UE_VLOG(this, LogSuss, Log, TEXT(" - TOTAL: %4.2f"), Score);
#endif

			if (!FMath::IsNearlyZero(Score))
			{
				CandidateActions.Add(FSussActionScoringResult { i, Ctx, Score });
			}
		}
		
	}

	ChooseActionFromCandidates();
	
}

void USussBrainComponent::ResolveParameters(AActor* Self,
	const TMap<FName, FSussParameter>& InParams,
	TMap<FName, FSussParameter>& OutParams)
{
	FSussContext SelfContext { Self };
	for (const auto& Param : InParams)
	{
		OutParams.Add(Param.Key, ResolveParameter(SelfContext, Param.Value));
	}
}

FSussParameter USussBrainComponent::ResolveParameter(const FSussContext& SelfContext, const FSussParameter& Value) const
{
	static TMap<FName, FSussParameter> DummyParams;

	if (Value.Type == ESussParamType::AutoParameter)
	{
		auto SUSS = GetSUSS(GetWorld());
		if (Value.InputOrParameterTag.MatchesTag(TAG_SussInputParentTag))
		{
			// Inputs always resolve to float
			if (auto InputProvider = SUSS->GetInputProvider(Value.InputOrParameterTag))
			{
				return InputProvider->Evaluate(this, SelfContext, DummyParams);
			}
		}
		else if (Value.InputOrParameterTag.MatchesTag(TAG_SussParamParentTag))
		{
			// Other auto params can return any value
			if (auto ParamProvider = SUSS->GetParameterProvider(Value.InputOrParameterTag))
			{
				return ParamProvider->Evaluate(this, SelfContext, DummyParams);
			}
		}
	}
	// Fallback
	return Value;
}

void USussBrainComponent::GenerateContexts(AActor* Self, const FSussActionDef& Action, TArray<FSussContext>& OutContexts)
{
	auto SUSS = GetSUSS(GetWorld());

	auto Pool = GetSussPool(GetWorld());

	if (Action.Queries.Num() > 0)
	{
		TSet<ESussQueryContextElement> ContextElements;
		TSet<FName> NamedQueryValues;

		for (const auto& Query : Action.Queries)
		{
			auto QueryProvider = SUSS->GetQueryProvider(Query.QueryTag);
			if (!QueryProvider)
				continue;

			FSussScopeReservedMap ResolvedQueryParamsScope = Pool->ReserveMap<FName, FSussParameter>();
			TMap<FName, FSussParameter>& ResolvedParams = *ResolvedQueryParamsScope.Get<FName, FSussParameter>();
			ResolveParameters(Self, Query.Params, ResolvedParams);

			// Because we use the results from each query to multiply combinations with existing results, we cannot have >1 query
			// returning the same element (you'd multiply Targets * Targets for example)
			const auto Element = QueryProvider->GetProvidedContextElement();
			// Special case for Named Values, we can have multiples, just not providing the same name
			if (Element != ESussQueryContextElement::NamedValue && ContextElements.Contains(Element))
			{
				UE_LOG(LogSuss,
				       Warning,
				       TEXT("Action %s has more than one query returning %s, ignoring extra one %s"),
				       *Action.ActionTag.ToString(),
				       *StaticEnum<ESussQueryContextElement>()->GetValueAsString(Element),
				       *Query.QueryTag.ToString())
				continue;
			}
			ContextElements.Add(Element);

			if (Element == ESussQueryContextElement::NamedValue)
			{
				if (auto NQP = Cast<USussNamedValueQueryProvider>(QueryProvider))
				{
					const FName ValueName = NQP->GetQueryValueName();
					// Make sure we haven't seen this name before; since we allow multiple named type queries
					if (NamedQueryValues.Contains(ValueName))
					{
						UE_LOG(LogSuss,
							   Warning,
							   TEXT("Action %s has more than one query returning named value %s, ignoring extra one %s"),
							   *Action.ActionTag.ToString(),
							   *ValueName.ToString(),
							   *Query.QueryTag.ToString());
						continue;
					}
					NamedQueryValues.Add(ValueName);
				}
			}

			if (QueryProvider->IsCorrelatedWithContext())
			{
				IntersectCorrelatedContexts(Self, Query, QueryProvider, ResolvedParams, OutContexts);
			}
			else
			{
				AppendUncorrelatedContexts(Self, Query, QueryProvider, ResolvedParams, OutContexts);
			}
		
			
		}
	}
	else
	{
		// No queries, just self
		OutContexts.Add(FSussContext { Self });
	}
	
}

void USussBrainComponent::IntersectCorrelatedContexts(AActor* Self,
                                                   const FSussQuery& Query,
                                                   USussQueryProvider* QueryProvider,
                                                   const TMap<FName, FSussParameter>& Params,
                                                   TArray<FSussContext>& InOutContexts)
{
	// Correlated results run a query once for each existing context generated from previous queries, then combine the
	// results with that one context, meaning that instead of C * N contexts, you get N(C1) + N(C2) + .. N(Cx) contexts

	auto Pool = GetSussPool(GetWorld());
	const auto Element = QueryProvider->GetProvidedContextElement();

	int InContextCount = InOutContexts.Num();

	for (int i = 0; i < InContextCount; ++i)
	{
		FSussContext& SourceContext = InOutContexts[i];
		int NumResults = 0;
		switch(Element)
		{
		case ESussQueryContextElement::Target:
			{
				FSussScopeReservedArray Targets = Pool->ReserveArray<TWeakObjectPtr<AActor>>();
				QueryProvider->GetResultsInContext<TWeakObjectPtr<AActor>>(this, Self, SourceContext, Params, *Targets.Get<TWeakObjectPtr<AActor>>());

				NumResults = Targets.Get<TWeakObjectPtr<AActor>>()->Num();
				if (NumResults > 0)
				{
					AppendCorrelatedContexts<TWeakObjectPtr<AActor>>(Self,
					                                                 Targets,
					                                                 SourceContext,
					                                                 InOutContexts,
					                                                 [](const TWeakObjectPtr<AActor>& Target,
					                                                    FSussContext& Ctx)
					                                                 {
						                                                 Ctx.Target = Target;
					                                                 });
				}
				break;
			}
		case ESussQueryContextElement::Location:
			{
				FSussScopeReservedArray Targets = Pool->ReserveArray<FVector>();
				QueryProvider->GetResultsInContext<FVector>(this, Self, SourceContext, Params, *Targets.Get<FVector>());

				NumResults = Targets.Get<FVector>()->Num();
				if (NumResults > 0)
				{
					AppendCorrelatedContexts<FVector>(Self,
													  Targets,
													  SourceContext,
													  InOutContexts,
													  [](const FVector& Location, FSussContext& Ctx)
													  {
														  Ctx.Location = Location;
													  });
				}
				break;
			}
		case ESussQueryContextElement::NamedValue:
			{
				if (auto NQP = Cast<USussNamedValueQueryProvider>(QueryProvider))
				{
					const FName ValueName = NQP->GetQueryValueName();
					FSussScopeReservedArray NamedValues = Pool->ReserveArray<FSussContextValue>();
					QueryProvider->GetResultsInContext<FSussContextValue>(this, Self, SourceContext, Params, *NamedValues.Get<FSussContextValue>());
					NumResults = NamedValues.Get<FSussContextValue>()->Num();
					if (NumResults > 0)
					{
						AppendCorrelatedContexts<FSussContextValue>(Self,
																	NamedValues,
																	SourceContext,
																	InOutContexts,
																	[ValueName](const FSussContextValue& Value,
																				FSussContext& Ctx)
																	{
																		Ctx.NamedValues.Add(ValueName, Value);
																	});
					}
				}
				break;
			}
		}

		if (NumResults == 0)
		{
			// Correlated queries require results from BOTH (intersection). If this query didn't return any
			// results, it means that we must remove this incoming context because it's not valid
			InOutContexts.RemoveAt(i);
			--i;
			--InContextCount;
		}
	}
}

void USussBrainComponent::AppendUncorrelatedContexts(AActor* Self,
                                                     const FSussQuery& Query,
                                                     USussQueryProvider* QueryProvider,
                                                     const TMap<FName, FSussParameter>& Params,
                                                     TArray<FSussContext>& OutContexts)
{
	// Uncorrelated results run a query once, and combine the results in every combination with any existing

	auto Pool = GetSussPool(GetWorld());
	const auto Element = QueryProvider->GetProvidedContextElement();
	switch (Element)
	{
	case ESussQueryContextElement::Target:
		{
			FSussScopeReservedArray Targets = Pool->ReserveArray<TWeakObjectPtr<AActor>>();
			Targets.Get<TWeakObjectPtr<AActor>>()->Append(
				QueryProvider->GetResults<TWeakObjectPtr<AActor>>(this, Self, Query.MaxFrequency, Params));
			AppendUncorrelatedContexts<TWeakObjectPtr<AActor>>(Self,
			                                       Targets,
			                                       OutContexts,
			                                       [](const TWeakObjectPtr<AActor>& Target, FSussContext& Ctx)
			                                       {
				                                       Ctx.Target = Target;
			                                       });
			break;
		}
	case ESussQueryContextElement::Location:
		{
			FSussScopeReservedArray Locations = Pool->ReserveArray<FVector>();
			Locations.Get<FVector>()->Append(
				QueryProvider->GetResults<FVector>(this, Self, Query.MaxFrequency, Params));
			AppendUncorrelatedContexts<FVector>(Self,
			                        Locations,
			                        OutContexts,
			                        [](const FVector& Loc, FSussContext& Ctx)
			                        {
				                        Ctx.Location = Loc;
			                        });

			break;
		}
	case ESussQueryContextElement::NamedValue:
		{
			if (auto NQP = Cast<USussNamedValueQueryProvider>(QueryProvider))
			{
				const FName ValueName = NQP->GetQueryValueName();
				FSussScopeReservedArray NamedValues = Pool->ReserveArray<FSussContextValue>();
				NamedValues.Get<FSussContextValue>()->Append(
					QueryProvider->GetResults<FSussContextValue>(this, Self, Query.MaxFrequency, Params));
				AppendUncorrelatedContexts<FSussContextValue>(Self,
				                                  NamedValues,
				                                  OutContexts,
				                                  [ValueName](const FSussContextValue& Value, FSussContext& Ctx)
				                                  {
					                                  Ctx.NamedValues.Add(ValueName, Value);
				                                  });
			}
			break;
		}
	}
}


AAIController* USussBrainComponent::GetAIController() const
{
	if (const auto Cached = AiController.Get())
	{
		return Cached;
	}

	AAIController* Found = Cast<AAIController>(GetOwner());
	if (!Found)
	{
		if (const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			Found = Cast<AAIController>(Pawn->GetController());
		}
	}

	if (Found)
	{
		AiController = Found;
		return Found;
	}

	return nullptr;
	
}

UCharacterMovementComponent* USussBrainComponent::GetCharacterMovement() const
{
	if (const ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		return Char->GetCharacterMovement();
	}

	return nullptr;
	
}

APawn* USussBrainComponent::GetPawn() const
{
	if (auto AI = GetAIController())
	{
		return AI->GetPawn();
	}
	return nullptr;
}

AActor* USussBrainComponent::GetSelf() const
{
	if (auto Ctrl = GetAIController())
	{
		return Ctrl->GetPawn();
	}

	// Fallback support for brains directly on actor (mostly for testing)
	return GetOwner();
}

double USussBrainComponent::GetTimeSinceActionPerformed(FGameplayTag ActionTag) const
{
	if (auto pTime = ActionsTimeLastPerformed.Find(ActionTag))
	{
		return GetWorld()->GetTimeSeconds() - *pTime;
	}

	return 9999999.9;
}

void USussBrainComponent::OnPerceptionUpdated(const TArray<AActor*>& Actors)
{
	QueueForUpdate();
}

FString USussBrainComponent::GetDebugSummaryString() const
{
	TStringBuilder<256> Builder;
	Builder.Appendf(TEXT("Distance Category: %s  UpdateFreq: %4.2f\n"), *StaticEnum<ESussDistanceCategory>()->GetValueAsString(DistanceCategory), CurrentUpdateInterval);
	
	if (CombinedActionsByPriority.IsValidIndex(CurrentActionResult.ActionDefIndex))
	{
		// Log all actions
		// Log all considerations?
		const FSussActionDef& Def = CombinedActionsByPriority[CurrentActionResult.ActionDefIndex];
		Builder.Appendf(
			TEXT(
				"Current Action: {yellow}%s{white} Score: {yellow}%4.2f{white}\n"
				"Inertia: {yellow}%4.2f{white}"),
				Def.Description.IsEmpty() ? 
					*CurrentActionInstance->GetClass()->GetName() :
					*Def.Description,
				CurrentActionResult.Score,
				CurrentActionInertia);
	}

	return Builder.ToString();
		
}

void USussBrainComponent::DebugLocations(TArray<FVector>& OutLocations, bool bIncludeDetails) const
{
	if (IsValid(CurrentActionInstance))
	{
		CurrentActionInstance->DebugLocations(OutLocations, bIncludeDetails);
	}
}

void USussBrainComponent::GetDebugDetailLines(TArray<FString>& OutLines) const
{
	OutLines.Reset();
	OutLines.Add(TEXT("Candidate Actions:"));
	for (const auto& Action : CandidateActions)
	{
		const FSussActionDef& Def = CombinedActionsByPriority[Action.ActionDefIndex];
		OutLines.Add(FString::Printf(
			TEXT(" - {yellow}%s  {white}%4.2f"),
			Def.Description.IsEmpty()
				? *Def.ActionTag.ToString()
				: *Def.Description,
			Action.Score
			));

		// If we want to list consideration scores here, we have to store them
	}
}
