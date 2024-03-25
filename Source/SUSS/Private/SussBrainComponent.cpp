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


// Sets default values for this component's properties
USussBrainComponent::USussBrainComponent(): bQueuedForUpdate(false),
                                            TimeSinceLastUpdate(0),
                                            BrainConfigAsset(nullptr),
                                            CachedUpdateRequestTime(1),
                                            CurrentAction()
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
	
}


void USussBrainComponent::StartLogic()
{
	Super::StartLogic();

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
	if (!bQueuedForUpdate && TimeSinceLastUpdate > CachedUpdateRequestTime)
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
	if (IsValid(CurrentAction.ActionInstance))
	{
		CurrentAction.ActionInstance->CancelAction(Interrupter);
		CurrentAction.ActionInstance = nullptr;
	}
}

void USussBrainComponent::ChooseAction(const FSussActionScoringResult& ActionResult)
{
	checkf(ActionResult.Def, TEXT("No supplied action def"));
	checkf(IsValid(ActionResult.Def->ActionClass), TEXT("Action class not valid"));

	TSubclassOf<USussAction> PreviousActionClass = nullptr;
	if (IsValid(CurrentAction.ActionInstance))
	{
		PreviousActionClass = CurrentAction.Def->ActionClass;
	}
	StopCurrentAction();
	CurrentAction = ActionResult;
	CurrentActionInertiaCooldown = CurrentAction.Def->InertiaCooldown;

	// Note that to allow BP classes we need to construct using the default object
	CurrentAction.ActionInstance = GetSussPool(GetWorld())->ReserveAction(CurrentAction.Def->ActionClass, this, CurrentAction.Def->ActionClass->GetDefaultObject());
	CurrentAction.ActionInstance->Init(this, CurrentAction.Context);
	CurrentAction.ActionInstance->InternalOnActionCompleted.BindUObject(this, &USussBrainComponent::OnActionCompleted);
	ActionNamesTimeLastPerformed.Add(ActionResult.Def->ActionClass->GetFName(), GetWorld()->GetTimeSeconds());
	
	CurrentAction.ActionInstance->PerformAction(ActionResult.Context, PreviousActionClass);

	
}

void USussBrainComponent::OnActionCompleted(USussAction* SussAction)
{
	if (IsValid(CurrentAction.ActionInstance))
	{
		checkf(CurrentAction.ActionInstance == SussAction, TEXT("OnActionCompleted called from action which was not current!"))
		GetSussPool(GetWorld())->FreeAction(CurrentAction.ActionInstance);
		CurrentAction.ActionInstance = nullptr;
		CurrentActionInertiaCooldown = 0;
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Received an OnActionCompleted callback on %s from action type %s when no current action is set"), *GetName(), *SussAction->GetClass()->GetName());
	}

	SussAction->InternalOnActionCompleted.Unbind();
}

void USussBrainComponent::Update()
{
	if (!GetOwner()->HasAuthority())
		return;

	if (CombinedActionsByPriority.IsEmpty())
		return;

	auto SUSS = GetSUSS(GetWorld());
	auto Pool = GetSussPool(GetWorld());
	AActor* Self = GetSelf();

	if (IsValid(CurrentAction.ActionInstance) && CurrentAction.Def->Inertia > 0 && CurrentAction.Def->InertiaCooldown > 0)
	{
		CurrentActionInertia = CurrentAction.Def->Inertia * (CurrentActionInertiaCooldown / CurrentAction.Def->InertiaCooldown);
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
		if (!IsValid(NextAction.ActionClass) || !USussUtility::IsActionEnabled(NextAction.ActionClass))
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
		
		// Evaluate this action for every applicable context
		for (const auto& Ctx : Contexts)
		{
			float Score = NextAction.Weight;
			for (auto& Consideration : NextAction.Considerations)
			{
				if (auto InputProvider = SUSS->GetInputProvider(Consideration.InputTag))
				{
					// Resolve parameters
					FSussScopeReservedMap ResolvedQueryParamsScope = Pool->ReserveMap<FName, FSussParameter>();
					TMap<FName, FSussParameter>& ResolvedParams = *ResolvedQueryParamsScope.Get<FName, FSussParameter>();
					ResolveParameters(Self, Consideration.Parameters, ResolvedParams);

					const float RawInputValue = InputProvider->Evaluate(Ctx, ResolvedParams);

					// Normalise to bookends and clamp
					const float NormalisedInput = FMath::Clamp(FMath::GetRangePct(
						                                           ResolveParameterToFloat(
							                                           Ctx,
							                                           Consideration.BookendMin),
						                                           ResolveParameterToFloat(
							                                           Ctx,
							                                           Consideration.BookendMax),
						                                           RawInputValue),
					                                           0.f,
					                                           1.f);

					// Transform through curve
					const float ConScore = Consideration.EvaluateCurve(NormalisedInput);

					// Accumulate with overall score
					Score *= ConScore;

					// Early-out if we've ended up at zero, nothing can change this now
					if (FMath::IsNearlyZero(Score))
					{
						break;
					}
					
				}
			}

			if (!FMath::IsNearlyZero(Score))
			{
				// This is a possible choice, but check that it exceeds inertia
				if (!IsValid(CurrentAction.ActionInstance) || Score > CurrentAction.Score + CurrentActionInertia)
				{
					CandidateActions.Add(FSussActionScoringResult { &NextAction, Ctx, Score });
				}
			}
		}
		
	}

	ChooseActionFromCandidates();
	
	bQueuedForUpdate = false;
	TimeSinceLastUpdate = 0;
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

float USussBrainComponent::ResolveParameterToFloat(const FSussContext& SelfContext, const FSussParameter& Value) const
{
	// No additional parameters
	static TMap<FName, FSussParameter> DummyParams;
	
	if (Value.Type == ESussParamType::Input)
	{
		auto SUSS = GetSUSS(GetWorld());
		if (auto InputProvider = SUSS->GetInputProvider(Value.InputTag))
		{
			return InputProvider->Evaluate(SelfContext, DummyParams);
		}
	}
	return Value.FloatValue;
}

FSussParameter USussBrainComponent::ResolveParameter(const FSussContext& SelfContext, const FSussParameter& Value) const
{
	return FSussParameter(ResolveParameterToFloat(SelfContext, Value));
}

void USussBrainComponent::GenerateContexts(AActor* Self, const FSussActionDef& Action, TArray<FSussContext>& OutContexts)
{
	auto SUSS = GetSUSS(GetWorld());

	auto Pool = GetSussPool(GetWorld());
	FSussScopeReservedArray Targets = Pool->ReserveArray<TWeakObjectPtr<AActor>>();
	FSussScopeReservedArray Locations = Pool->ReserveArray<FVector>();
	FSussScopeReservedArray Rotations = Pool->ReserveArray<FRotator>();
	FSussScopeReservedArray CustomValues = Pool->ReserveArray<TSussContextValue>();

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
		case ESussQueryContextElement::CustomValue:
			CustomValues.Get<TSussContextValue>()->Append(QueryProvider->GetResults<TSussContextValue>(this, Self, Query.MaxFrequency, ResolvedParams));
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
	AppendContexts<TSussContextValue>(Self, CustomValues,
	                                  OutContexts,
	                                  [](const TSussContextValue& CV, FSussContext& Ctx)
	                                  {
		                                  Ctx.Custom = CV;
	                                  });

	if(OutContexts.IsEmpty())
	{
		// We had no dimensions, just self
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

double USussBrainComponent::GetTimeSinceActionPerformed(TSubclassOf<USussAction> ActionClass) const
{
	if (auto pTime = ActionNamesTimeLastPerformed.Find(ActionClass->GetFName()))
	{
		return GetWorld()->GetTimeSeconds() - *pTime;
	}

	return 9999999.9;
}

FString USussBrainComponent::GetDebugInfoString() const
{
	if (IsValid(CurrentAction.ActionInstance))
	{
		return FString::Printf(
			TEXT(
				"Current Action: {yellow}%s{white} Score: {yellow}%4.2f{white}\n"
				"Countdown: {yellow}%4.2f{white}\n"
				"Inertia: {yellow}%4.2f{white}"),
				*CurrentAction.ActionInstance->GetClass()->GetName(), CurrentAction.Score,
				CurrentActionInertia,
				FMath::Max(0.0f, CachedUpdateRequestTime - TimeSinceLastUpdate));
	}

	return FString();
		
}

void USussBrainComponent::DebugLocations(TArray<FVector>& OutLocations) const
{
	if (IsValid(CurrentAction.ActionInstance))
	{
		CurrentAction.ActionInstance->DebugLocations(OutLocations);
	}
}
