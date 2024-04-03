// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussInputProvider.h"
#include "SussBlackboardInputProviders.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputBlackboardFloat);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputBlackboardBool);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputBlackboardAuto);
/**
 * Gets a float value from the blackboard.
 * Requires a parameter of "Key" of type FName. Blackboard entry must me a boolean value.
 */
UCLASS()
class SUSS_API USussBlackboardFloatInputProvider : public USussInputProvider
{
	GENERATED_BODY()
	
public:

	USussBlackboardFloatInputProvider();
	
	virtual float Evaluate_Implementation(const USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Gets a boolean value from the blackboard, as a value of 1 for true and 0 for false.
 * Requires a parameter of "Key" of type FName. Blackboard entry must me a float value.
 */
UCLASS()
class SUSS_API USussBlackboardBoolInputProvider : public USussInputProvider
{
	GENERATED_BODY()
	
public:

	USussBlackboardBoolInputProvider();
	
	virtual float Evaluate_Implementation(const USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * Gets a value from the blackboard, automatically converting to float from any compatible type (float/int/bool).
 * Requires a parameter of "Key" of type FName. 
 */
UCLASS()
class SUSS_API USussBlackboardAutoInputProvider : public USussInputProvider
{
	GENERATED_BODY()
	
public:

	USussBlackboardAutoInputProvider();
	
	virtual float Evaluate_Implementation(const USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};
