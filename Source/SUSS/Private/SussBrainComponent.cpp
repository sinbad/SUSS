#include "SussBrainComponent.h"

#include "AIController.h"
#include "SussAction.h"
#include "SussCommon.h"
#include "SussSettings.h"
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

void USussBrainComponent::ChooseAction(const FActionScoringResult& ActionResult)
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
	
	// - Iterate priority groups by descending priority
	//   - Iterate actions
	//     - Check globally disabled action classes
	//     - Check required / blocking tags
	//     - Init score accumulator to Weight
	//     - If score == 0, early-out
	//     - Iterate considerations
	//         - Generate contexts from input
	//         - Iterate contexts
	//           - Update non-literal params
	//           - Evaluate input
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

