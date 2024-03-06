#include "SussBrainComponent.h"

#include "AIController.h"
#include "SussAction.h"
#include "SussCommon.h"
#include "SussGameSubsystem.h"
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

		const TArray<FSussContext>& Contexts = GenerateContexts(NextAction);
		
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

const TArray<FSussContext>& USussBrainComponent::GenerateContexts(const FSussActionDef& Action)
{
	auto SUSS = GetSUSS(GetWorld());
	
	// Figure out what queries we need first
	USussTargetQueryProvider* TargetQ = nullptr;
	USussLocationQueryProvider* LocationQ = nullptr;
	USussRotationQueryProvider* RotationQ = nullptr;
	USussCustomValueQueryProvider* CustomQ = nullptr;
	
	for (const auto& Con : Action.Considerations)
	{
		auto Input = SUSS->GetInputProvider(Con.InputTag);
		if (!Input)
			continue;

		const FGameplayTagContainer& QueryTags = Input->GetRequestedQueryTags();
		for (auto& QTag : QueryTags.GetGameplayTagArray())
		{
			if (auto QueryProv = SUSS->GetQueryProvider(QTag))
			{
				switch (QueryProv->GetProvidedContextElement())
				{
				case ESussQueryContextElement::Target:
					{
						if (auto NewTQ = Cast<USussTargetQueryProvider>(QueryProv))
						{
							if (TargetQ && TargetQ != NewTQ)
							{
								UE_LOG(LogSuss,
								       Error,
								       TEXT(
									       "Action is attempting to use 2 different target queries at once! Action Class: %s Target queries: %s, %s"
								       ),
								       *Action.ActionClass->GetName(),
								       *TargetQ->GetQueryTag().ToString(),
								       *QTag.ToString());
								continue;
							}

							TargetQ = NewTQ;
						}
						else
						{
							UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide target information but isn't subclassed from SussTargetQueryProvider"), *QueryProv->GetClass()->GetName());
						}
					}
					break;
				case ESussQueryContextElement::Location:
					{
						if (auto NewLQ = Cast<USussLocationQueryProvider>(QueryProv))
						{
							if (LocationQ && LocationQ != NewLQ)
							{
								UE_LOG(LogSuss,
									   Error,
									   TEXT(
										   "Action is attempting to use 2 different location queries at once! Action Class: %s Target queries: %s, %s"
									   ),
									   *Action.ActionClass->GetName(),
									   *LocationQ->GetQueryTag().ToString(),
									   *QTag.ToString());
								continue;
							}

							LocationQ = NewLQ;
						}
						else
						{
							UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide target information but isn't subclassed from SussTargetQueryProvider"), *QueryProv->GetClass()->GetName());
						}
					}
					break;
				case ESussQueryContextElement::Rotation:
					{
						if (auto NewRQ = Cast<USussRotationQueryProvider>(QueryProv))
						{
							if (RotationQ && RotationQ != NewRQ)
							{
								UE_LOG(LogSuss,
									   Error,
									   TEXT(
										   "Action is attempting to use 2 different location queries at once! Action Class: %s Target queries: %s, %s"
									   ),
									   *Action.ActionClass->GetName(),
									   *RotationQ->GetQueryTag().ToString(),
									   *QTag.ToString());
								continue;
							}

							RotationQ = NewRQ;
						}
						else
						{
							UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide target information but isn't subclassed from SussTargetQueryProvider"), *QueryProv->GetClass()->GetName());
						}
					}
					break;
				case ESussQueryContextElement::CustomValue:
					{
						if (auto NewCQ = Cast<USussCustomValueQueryProvider>(QueryProv))
						{
							if (CustomQ && CustomQ != NewCQ)
							{
								UE_LOG(LogSuss,
									   Error,
									   TEXT(
										   "Action is attempting to use 2 different location queries at once! Action Class: %s Target queries: %s, %s"
									   ),
									   *Action.ActionClass->GetName(),
									   *CustomQ->GetQueryTag().ToString(),
									   *QTag.ToString());
								continue;
							}

							CustomQ = NewCQ;
						}
						else
						{
							UE_LOG(LogSuss, Error, TEXT("Query provider %s claims to provide target information but isn't subclassed from SussTargetQueryProvider"), *QueryProv->GetClass()->GetName());
						}
					}
					break;
				}
			}
		}
	}

	// Get query results

			
	
	// Enumerate contexts from all considerations
	// Note: this gives us all possible contexts from all considerations, and we evaluate each consideration with all
	// contexts. It's possible that consideration 1 generates 3 contexts, but consideration 2 only needs one, because the
	// value it uses is common to all 3. In order to only call consideration 2 once, and re-use that result for the 3
	// calls to consideration 1, means being able to determine that context 1.A-B are all subsets of context 2.A.
	// But what if they're not subsets? What if the only thing they share is Self, and the other parameters
	// generate 3 contexts each but they're different? Clearly the other considerations don't care about these other
	// contexts, so we shouldn't create 3x3 contexts because 6 of them will be useless to each.
	// Really what we need is a set of contexts per consideration, which calculates the score. Then we combine the
	// contexts?


	return TempConsiderationContexts;
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

