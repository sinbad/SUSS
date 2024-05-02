
#include "../../Public/Inputs/SussPerceptionInputProviders.h"

#include "SussBrainComponent.h"
#include "SussUtility.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputSelfSightRange, "Suss.Input.Perception.Sight.RangeSelf", "Get the sight range of the controlled actor")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputSelfHearingRange, "Suss.Input.Perception.Hearing.RangeSelf", "Get the hearing range of the controlled actor")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLineOfSightToTarget, "Suss.Input.Perception.Sight.LineOfSightToTarget", "1 if the agent has line of sight to the target context, 0 if not")

USussSelfSightRangeInputProvider::USussSelfSightRangeInputProvider()
{
	InputTag = TAG_SussInputSelfSightRange;
}

float USussSelfSightRangeInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                const FSussContext& Ctx,
                                                                const TMap<FName, FSussParameter>& Parameters) const
{
	if (const auto Percept = Brain->GetPerceptionComponent())
	{
		if (const auto SenseCfg = Cast<UAISenseConfig_Sight>(Percept->GetSenseConfig(GetDefault<UAISense_Sight>()->GetSenseID())))
		{
			// Use the outer lose sight radius
			return SenseCfg->LoseSightRadius;
		}
	}

	return 1000;
}

USussSelfHearingRangeInputProvider::USussSelfHearingRangeInputProvider()
{
	InputTag = TAG_SussInputSelfHearingRange;
}

float USussSelfHearingRangeInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                  const FSussContext& Ctx,
                                                                  const TMap<FName, FSussParameter>& Parameters) const
{
	if (const auto Percept = Brain->GetPerceptionComponent())
	{
		if (const auto SenseCfg = Cast<UAISenseConfig_Hearing>(Percept->GetSenseConfig(GetDefault<UAISense_Hearing>()->GetSenseID())))
		{
			// Use the outer lose sight radius
			return SenseCfg->HearingRange;
		}
	}

	return 100;
}

USussLineOfSightToTargetInputProvider::USussLineOfSightToTargetInputProvider()
{
	InputTag = TAG_SussInputLineOfSightToTarget;
}

float USussLineOfSightToTargetInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	const auto Pawn = Brain->GetPawn();
	if (Pawn && Context.Target.IsValid())
	{
		UWorld* World = Pawn->GetWorld();
		FVector Start, End;
		FRotator DummyRot;
		Pawn->GetActorEyesViewPoint(Start, DummyRot);
		End = Context.Target->GetActorLocation();
		ECollisionChannel Channel = USussUtility::GetLineOfSightTraceChannel();

		FCollisionQueryParams Params(SCENE_QUERY_STAT(LineOfSight), true, Pawn);
		Params.AddIgnoredActor(Context.Target.Get());
		FHitResult Hit;
		bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, Channel, Params);

#if ENABLE_VISUAL_LOG
		UE_VLOG_ARROW(Brain->GetLogOwner(), LogSuss, Verbose, Start, End, bHit ? FColor::Red : FColor::Green, TEXT(""));
		if (bHit)
		{
			UE_VLOG_LOCATION(Brain->GetLogOwner(), LogSuss, Verbose, Hit.Location, 15, FColor::Red, TEXT(""));
		}
#endif

		return bHit ? 0 : 1;
	}

	return 0;
}
