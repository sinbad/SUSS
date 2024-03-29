// 

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "SussInputProvider.h"
#include "SussGameplayAttributeInputProvider.generated.h"

/**
 * A base input provider class that supplies the value of an attribute (not usable directly, see subclasses)
 */
UCLASS(Abstract)
class SUSS_API USussGameplayAttributeInputProvider : public USussInputProvider
{
	GENERATED_BODY()
public:
	/// The gameplay attribute which we want to access
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute Attribute;

	/// Optional attribute for the "Max" value of the attribute, for calculating normalised values
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute MaxAttribute;

	/// If the MaxAttribute setting is provided, whether to normalise the result to 0..1
	UPROPERTY(EditDefaultsOnly)
	bool bNormaliseIfMaxAttributeSet = true;

protected:

	float GetAttributeValue(const AActor* FromActor) const;
};

/**
 * An input provider that supplies the value of an attribute from "Self", ie the owner of a brain
 */
UCLASS()
class SUSS_API USussGameplayAttributeSelfInputProvider : public USussGameplayAttributeInputProvider
{
	GENERATED_BODY()
public:
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};
/**
 * An input provider that supplies the value of an attribute from a Target in a context
 */
UCLASS()
class SUSS_API USussGameplayAttributeTargetInputProvider : public USussGameplayAttributeInputProvider
{
	GENERATED_BODY()
public:
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};