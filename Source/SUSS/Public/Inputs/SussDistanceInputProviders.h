// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussInputProvider.h"
#include "SussDistanceInputProviders.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputTargetDistance);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputLocationDistance);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputTargetDistance2D);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputLocationDistance2D);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputTargetDistancePath);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputLocationDistancePath);

/**
 * Input that provides the distance from the AI to a given target 
 */
UCLASS()
class SUSS_API USussTargetDistanceInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussTargetDistanceInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input that provides the distance from the AI to a given location 
 */
UCLASS()
class SUSS_API USussLocationDistanceInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussLocationDistanceInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input that provides the 2D (XY) distance from the AI to a given target 
 */
UCLASS()
class SUSS_API USussTargetDistance2DInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussTargetDistance2DInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input that provides the 2D (XY) distance from the AI to a given location 
 */
UCLASS()
class SUSS_API USussLocationDistance2DInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussLocationDistance2DInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input that provides the distance from the AI to a given target along available paths 
 */
UCLASS()
class SUSS_API USussTargetDistancePathInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussTargetDistancePathInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Input that provides the distance from the AI to a given location along available paths
 */
UCLASS()
class SUSS_API USussLocationDistancePathInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	USussLocationDistancePathInputProvider();
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};
