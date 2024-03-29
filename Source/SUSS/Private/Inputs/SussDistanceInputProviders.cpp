
#include "Inputs/SussDistanceInputProviders.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTargetDistance, "Suss.Input.Distance.ToTarget", "Get the 3D distance to a target")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLocationDistance, "Suss.Input.Distance.ToLocation", "Get the 3D distance to a target")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputTargetDistance2D, "Suss.Input.Distance.ToTarget2D", "Get the 2D (XY) distance to a target")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputLocationDistance2D, "Suss.Input.Distance.ToLocation2D", "Get the 2D (XY) distance to a target")

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
	return FVector::Dist2D(
		Ctx.ControlledActor ? Ctx.ControlledActor->GetActorLocation() : FVector::ZeroVector,
		Ctx.Target.IsValid() ? Ctx.Target->GetActorLocation() : FVector::ZeroVector);
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
