#include "SussBrainComponent.h"

#include "AIController.h"
#include "SussAction.h"
#include "SussCommon.h"
#include "SussGameSubsystem.h"
#include "SussPoolSubsystem.h"
#include "SussSettings.h"
#include "SussUtility.h"
#include "SussWorldSubsystem.h"


// Sets default values for this component's properties
USussBrainComponent::USussBrainComponent(): bQueuedForUpdate(false), TimeSinceLastUpdate(0), CachedUpdateRequestTime(1)
{
	// Brains tick in order to queue themselves for update regularly
	PrimaryComponentTick.bCanEverTick = true;

	if (auto Settings = GetDefault<USussSettings>())
	{
		CachedUpdateRequestTime = Settings->BrainUpdateRequestIntervalSeconds;
	}

}


// Called when the game starts
void USussBrainComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		InitActions();
	}
	else
	{
		// No need to tick on non-server
		SetComponentTickEnabled(false);
	}

}

void USussBrainComponent::InitActions()
{
	// Collate all the actions from referenced action sets, and actions only on this instance
	CombinedActionsByPriority.Empty();
	for (auto ActionSet : ActionSets)
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
	for (auto& Action : ActionDefs)
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

void USussBrainComponent::ChooseActionFromCandidates(const TArray<FSussActionScoringResult>& Candidates)
{
}

void USussBrainComponent::ChooseAction(const FSussActionScoringResult& ActionResult)
{
	checkf(!CurrentAction.IsSet(), TEXT("Trying to choose a new action before the previous one is cleared"));
	checkf(ActionResult.Def, TEXT("No supplied action def"));
	checkf(IsValid(ActionResult.Def->ActionClass), TEXT("Action class not valid"));
	
	CurrentAction = ActionResult;
	CurrentActionInertia = CurrentAction->Def->Inertia;
	CurrentActionInertiaCooldown = CurrentAction->Def->InertiaCooldown;

	if (auto CDO = CurrentAction->Def->ActionClass.GetDefaultObject())
	{
		CDO->InternalOnActionCompleted.BindUObject(this, &USussBrainComponent::OnActionCompleted);
		CDO->PerformAction(this, CurrentAction->Context);
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("CDO not valid in USussBrainComponent::ChooseAction"));
	}
	
}

void USussBrainComponent::OnActionCompleted(USussAction* SussAction)
{
	if (CurrentAction.IsSet())
	{
		checkf(CurrentAction->Def->ActionClass.GetDefaultObject() == SussAction, TEXT("OnActionCompleted called from action which was not current!"))
		CurrentAction.Reset();
		CurrentActionInertia = 0;
		CurrentActionInertiaCooldown = 0;
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Received an OnActionCompleted callback on %s when no current action is set"), *GetName());
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

		auto ArrayPool = GetSussArrayPool(GetWorld());
		
		FSussScopeReservedArray Contexts = ArrayPool->ReserveArray<FSussContext>();
		GenerateContexts(NextAction, *Contexts.Get<FSussContext>());
		
		float Score = NextAction.Weight;
		

		
		
		
	}
	if (!CandidateActions.IsEmpty())
	{
		ChooseActionFromCandidates(CandidateActions);
	}

	
	
	//     - Init score accumulator to Weight
	//     - If score == 0, early-out
	//     - Iterate considerations
	//         - Generate contexts from input
	//         - Iterate contexts
	//           - Evaluate input
	//				- Provide way for input to resolve non-literal params
	//           - Apply bookends
	//           - Apply curve
	//           - Multiply value with accumulator
	//			 - If score == 0, early-out
	//      - If combined action score > 0, push action & context & score on to result list
	//   - Post-filter the results to those that exceed the currently executing action + inertia value (which should cool down over time)
	//     - This means we need: a tick which reduces inertia, AND a notification of when an action is finished to remove inertia early
	//   - Pick a result to execute (top score, random top N etc)

	bQueuedForUpdate = false;
	TimeSinceLastUpdate = 0;
}

void USussBrainComponent::GenerateContexts(const FSussActionDef& Action, TArray<FSussContext>& OutContexts)
{
	auto SUSS = GetSUSS(GetWorld());

	auto ArrayPool = GetSussArrayPool(GetWorld());
	FSussScopeReservedArray Targets = ArrayPool->ReserveArray<TWeakObjectPtr<AActor>>();
	FSussScopeReservedArray Locations = ArrayPool->ReserveArray<FVector>();
	FSussScopeReservedArray Rotations = ArrayPool->ReserveArray<FRotator>();
	FSussScopeReservedArray CustomValues = ArrayPool->ReserveArray<TSussContextValue>();

	AActor* Self = GetSelf();
	
	for (const auto& Query : Action.Queries)
	{
		auto QueryProvider = SUSS->GetQueryProvider(Query.QueryTag);
		if (!QueryProvider)
			continue;
		
		switch (QueryProvider->GetProvidedContextElement())
		{
		case ESussQueryContextElement::Target:
			{
				if (auto TQ = Cast<USussTargetQueryProvider>(QueryProvider))
				{
					Targets.Get<TWeakObjectPtr<AActor>>()->Append(TQ->GetResults(this, Self, Query.Params));
				}
				else
				{
					UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide target information but isn't subclassed from SussTargetQueryProvider"), *QueryProvider->GetClass()->GetName());
				}
			}
			break;
		case ESussQueryContextElement::Location:
			{
				if (auto LQ = Cast<USussLocationQueryProvider>(QueryProvider))
				{
					Locations.Get<FVector>()->Append(LQ->GetResults(this, Self, Query.Params));
				}
				else
				{
					UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide location information but isn't subclassed from USussLocationQueryProvider"), *QueryProvider->GetClass()->GetName());
				}
			}
			break;
		case ESussQueryContextElement::Rotation:
			{
				if (auto RQ = Cast<USussRotationQueryProvider>(QueryProvider))
				{
					Rotations.Get<FRotator>()->Append(RQ->GetResults(this, Self, Query.Params));
				}
				else
				{
					UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide rotation information but isn't subclassed from USussRotationQueryProvider"), *QueryProvider->GetClass()->GetName());
				}
			}
			break;
		case ESussQueryContextElement::CustomValue:
			{
				if (auto CQ = Cast<USussCustomValueQueryProvider>(QueryProvider))
				{
					CustomValues.Get<TSussContextValue>()->Append(CQ->GetResults(this, Self, Query.Params));
				}
				else
				{
					UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide custom value information but isn't subclassed from USussCustomValueQueryProvider"), *QueryProvider->GetClass()->GetName());
				}
			}
			break;
		}
	}

	// Now we have all the dimensions, produce a context for every combination
	
	
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

APawn* USussBrainComponent::GetSelf() const
{
	if (auto Ctrl = GetAIController())
	{
		return Ctrl->GetPawn();
	}

	return nullptr;
}

