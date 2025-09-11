
#include "Inputs/SussDistanceInputProviders.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "SussBrainComponent.h"
#include "SussUtility.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTargetDistance, "Suss.Input.Distance.ToTarget", "Get the 3D distance to a target")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLocationDistance, "Suss.Input.Distance.ToLocation", "Get the 3D distance to a location")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTargetDistance2D, "Suss.Input.Distance.ToTarget2D", "Get the 2D (XY) distance to a target")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLocationDistance2D, "Suss.Input.Distance.ToLocation2D", "Get the 2D (XY) distance to a location")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTargetDistancePath, "Suss.Input.Distance.ToTargetPath", "Get the distance to a target along navmesh paths")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLocationDistancePath, "Suss.Input.Distance.ToLocationPath", "Get the distance to a location along navmesh paths")

USussTargetDistanceInputProvider::USussTargetDistanceInputProvider()
{
	InputTag = TAG_SussInputTargetDistance;
}

float USussTargetDistanceInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                const FSussContext& Ctx,
                                                                const TMap<FName, FSussParameter>& Parameters) const
{
	return FVector::Distance(
		Ctx.ControlledActor ? Ctx.ControlledActor->GetActorLocation() : FVector::ZeroVector,
		Ctx.Target.IsValid() ? Ctx.Target->GetActorLocation() : FVector::ZeroVector);
}

USussLocationDistanceInputProvider::USussLocationDistanceInputProvider()
{
	InputTag = TAG_SussInputLocationDistance;
}

float USussLocationDistanceInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                  const FSussContext& Ctx,
                                                                  const TMap<FName, FSussParameter>& Parameters) const
{
	return FVector::Distance(
		Ctx.ControlledActor ? Ctx.ControlledActor->GetActorLocation() : FVector::ZeroVector,
		Ctx.Location);
}

USussTargetDistance2DInputProvider::USussTargetDistance2DInputProvider()
{
	InputTag = TAG_SussInputTargetDistance2D;
}

float USussTargetDistance2DInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                  const FSussContext& Ctx,
                                                                  const TMap<FName, FSussParameter>& Parameters) const
{
	if (Ctx.Target.IsValid())
	{
		return FVector::Dist2D(
			Ctx.ControlledActor ? Ctx.ControlledActor->GetActorLocation() : FVector::ZeroVector,
			Ctx.Target->GetActorLocation());
	}
	else
	{
		return UE_BIG_NUMBER;
	}
}

USussLocationDistance2DInputProvider::USussLocationDistance2DInputProvider()
{
	InputTag = TAG_SussInputLocationDistance2D;
}

float USussLocationDistance2DInputProvider::Evaluate_Implementation(const class USussBrainComponent* Brain,
                                                                    const FSussContext& Ctx,
                                                                    const TMap<FName, FSussParameter>& Parameters) const
{
	return FVector::Dist2D(
		Ctx.ControlledActor ? Ctx.ControlledActor->GetActorLocation() : FVector::ZeroVector,
		Ctx.Location);
}

USussTargetDistancePathInputProvider::USussTargetDistancePathInputProvider()
{
	InputTag = TAG_SussInputTargetDistancePath;
}

DECLARE_CYCLE_STAT(TEXT("SUSS Target Distance Path Input"), STAT_SUSSTargetDistancePathInput, STATGROUP_SUSS);

float USussTargetDistancePathInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	SCOPE_CYCLE_COUNTER(STAT_SUSSTargetDistancePathInput);

	if (Context.Target.IsValid())
	{
		bool bAllowPartialPath = false;
		if (auto pAllowPartialPathParam = Parameters.Find(SUSS::AllowPartialPathParamName))
		{
			bAllowPartialPath = pAllowPartialPathParam->BoolValue;
		}

		return USussUtility::GetPathDistanceTo(Brain->GetAIController(), Context.Target->GetActorLocation(), bAllowPartialPath);
	}
	return BIG_NUMBER;
}

USussLocationDistancePathInputProvider::USussLocationDistancePathInputProvider()
{
	InputTag = TAG_SussInputLocationDistancePath;
}

DECLARE_CYCLE_STAT(TEXT("SUSS Location Distance Path Input"), STAT_SUSSLocationDistancePathInput, STATGROUP_SUSS);

float USussLocationDistancePathInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	SCOPE_CYCLE_COUNTER(STAT_SUSSLocationDistancePathInput);

	bool bAllowPartialPath = false;
	if (auto pAllowPartialPathParam = Parameters.Find(SUSS::AllowPartialPathParamName))
	{
		bAllowPartialPath = pAllowPartialPathParam->BoolValue;
	}

	return USussUtility::GetPathDistanceTo(Brain->GetAIController(), Context.Location, bAllowPartialPath);
}
