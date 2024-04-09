#include "SussBrainComponent.h"

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
#include "Perception/AIPerceptionComponent.h"


// Sets default values for this component's properties
USussBrainComponent::USussBrainComponent(): bQueuedForUpdate(false),
                                            TimeSinceLastUpdate(0),
                                            BrainConfigAsset(nullptr),
                                            CachedUpdateRequestTime(1),
                                            CurrentActionResult(),
                                            PerceptionComp(nullptr)
{
	// Brains tick in order to queue themselves for update regularly
	PrimaryComponentTick.bCanEverTick = true;

	// Disable ticking by default
	PrimaryComponentTick.SetTickFunctionEnable(false);

	if (auto Settings = GetDefault<USussSettings>())
	{
		CachedUpdateRequestTime = Settings->BrainUpdateRequestIntervalSeconds;
	}
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

	// Randomise the time that brains update to spread them out
	TimeSinceLastUpdate = FMath::RandRange(0.0f, CachedUpdateRequestTime);

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
	
	SetComponentTickEnabled(true);
	
}

void USussBrainComponent::StopLogic(const FString& Reason)
{
	Super::StopLogic(Reason);

	StopCurrentAction();
	SetComponentTickEnabled(false);
}

void USussBrainComponent::RestartLogic()
{
	Super::RestartLogic();

	StopCurrentAction();
	TimeSinceLastUpdate = 9999999;
	SetComponentTickEnabled(true);
}

void USussBrainComponent::PauseLogic(const FString& Reason)
{
	Super::PauseLogic(Reason);

	SetComponentTickEnabled(false);
}

EAILogicResuming::Type USussBrainComponent::ResumeLogic(const FString& Reason)
{
	auto Ret = Super::ResumeLogic(Reason);
	if (Ret != EAILogicResuming::RestartedInstead)
	{
		// restarted calls RestartLogic
		SetComponentTickEnabled(true);
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

// Called every frame
void USussBrainComponent::TickComponent(float DeltaTime,
                                        ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->HasAuthority())
	{
		if (CurrentActionInertiaCooldown > 0)
		{
			CurrentActionInertiaCooldown = FMath::Max(CurrentActionInertiaCooldown - DeltaTime, 0);
		}

		CheckForNeededUpdate(DeltaTime);
	}
}

void USussBrainComponent::CheckForNeededUpdate(float DeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;
	if (TimeSinceLastUpdate > CachedUpdateRequestTime)
	{
		QueueForUpdate();
	}
}

void USussBrainComponent::QueueForUpdate()
{
	if (!bQueuedForUpdate)
	{
		if (auto SS = GetSussWorldSubsystem(GetWorld()))
		{
			SS->QueueBrainUpdate(this);
			bQueuedForUpdate = true;
		}
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

	if (ActionResult.ActionDefIndex == CurrentActionResult.ActionDefIndex &&
		ActionResult.Context == CurrentActionResult.Context)
	{
		// We're already running it, do nothing
#if ENABLE_VISUAL_LOG
		UE_VLOG(this, LogSuss, Log, TEXT("No Action Change, same as current"));
#endif
		return;
	}

	auto SUSS = GetSUSS(GetWorld());
	const FSussActionDef& Def = CombinedActionsByPriority[ActionResult.ActionDefIndex];
	const TSubclassOf<USussAction> ActionClass = SUSS->GetActionClass(Def.ActionTag);

#if ENABLE_VISUAL_LOG
	UE_VLOG(this, LogSuss, Log, TEXT("Chose NEW action: %s"), Def.Description.IsEmpty() ? *Def.ActionTag.ToString() : *Def.Description);
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
	TimeSinceLastUpdate = 0;
	
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
			UE_VLOG(this, LogSuss, Log, TEXT(" - Context: %s"), *Ctx.ToString());
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
		FSussScopeReservedArray Targets = Pool->ReserveArray<TWeakObjectPtr<AActor>>();
		FSussScopeReservedArray Locations = Pool->ReserveArray<FVector>();
		FSussScopeReservedArray Rotations = Pool->ReserveArray<FRotator>();
		FSussScopeReservedArray Tags = Pool->ReserveArray<FGameplayTag>();
		FSussScopeReservedArray CustomValues = Pool->ReserveArray<TSussCustomContextValue>();

		for (const auto& Query : Action.Queries)
		{
			auto QueryProvider = SUSS->GetQueryProvider(Query.QueryTag);
			if (!QueryProvider)
				continue;

			FSussScopeReservedMap ResolvedQueryParamsScope = Pool->ReserveMap<FName, FSussParameter>();
			TMap<FName, FSussParameter>& ResolvedParams = *ResolvedQueryParamsScope.Get<FName, FSussParameter>();
			ResolveParameters(Self, Query.Params, ResolvedParams);
		
			switch (QueryProvider->GetProvidedContextElement())
			{
			case ESussQueryContextElement::Target:
				Targets.Get<TWeakObjectPtr<AActor>>()->Append(QueryProvider->GetResults<TWeakObjectPtr<AActor>>(this, Self, Query.MaxFrequency, ResolvedParams));
				break;
			case ESussQueryContextElement::Location:
				Locations.Get<FVector>()->Append(QueryProvider->GetResults<FVector>(this, Self, Query.MaxFrequency, ResolvedParams));
				break;
			case ESussQueryContextElement::Rotation:
				Rotations.Get<FRotator>()->Append(QueryProvider->GetResults<FRotator>(this, Self, Query.MaxFrequency, ResolvedParams));
				break;
			case ESussQueryContextElement::Tag:
				Tags.Get<FGameplayTag>()->Append(QueryProvider->GetResults<FGameplayTag>(this, Self, Query.MaxFrequency, ResolvedParams));
				break;
			case ESussQueryContextElement::CustomValue:
				CustomValues.Get<TSussCustomContextValue>()->Append(QueryProvider->GetResults<TSussCustomContextValue>(this, Self, Query.MaxFrequency, ResolvedParams));
				break;
			}
		}

		// Now we have all the dimensions, produce a context for every combination
		AppendContexts<TWeakObjectPtr<AActor>>(Self, Targets,
											   OutContexts,
											   [](const TWeakObjectPtr<AActor>& Target, FSussContext& Ctx)
											   {
												   Ctx.Target = Target;
											   });
		AppendContexts<FVector>(Self, Locations,
								OutContexts,
								[](const FVector& Loc, FSussContext& Ctx)
								{
									Ctx.Location = Loc;
								});
		AppendContexts<FRotator>(Self, Rotations,
								 OutContexts,
								 [](const FRotator& Rot, FSussContext& Ctx)
								 {
									 Ctx.Rotation = Rot;
								 });
		AppendContexts<FGameplayTag>(Self, Tags,
								 OutContexts,
								 [](const FGameplayTag& Tag, FSussContext& Ctx)
								 {
									 Ctx.Tag = Tag;
								 });
		AppendContexts<TSussCustomContextValue>(Self, CustomValues,
										  OutContexts,
										  [](const TSussCustomContextValue& CV, FSussContext& Ctx)
										  {
											  Ctx.Custom = CV;
										  });
	}
	else
	{
		// No queries, just self
		OutContexts.Add(FSussContext { Self });
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
	if (CombinedActionsByPriority.IsValidIndex(CurrentActionResult.ActionDefIndex))
	{
		// Log all actions
		// Log all considerations?
		const FSussActionDef& Def = CombinedActionsByPriority[CurrentActionResult.ActionDefIndex];
		return FString::Printf(
			TEXT(
				"Current Action: {yellow}%s{white} Score: {yellow}%4.2f{white}\n"
				"Countdown: {yellow}%4.2f{white}\n"
				"Inertia: {yellow}%4.2f{white}"),
				Def.Description.IsEmpty() ? 
					*CurrentActionInstance->GetClass()->GetName() :
					*Def.Description,
				CurrentActionResult.Score,
				FMath::Max(0.0f, CachedUpdateRequestTime - TimeSinceLastUpdate),
				CurrentActionInertia);
	}

	return FString();
		
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
