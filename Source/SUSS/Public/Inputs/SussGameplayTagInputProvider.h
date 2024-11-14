// 

#pragma once

#include "CoreMinimal.h"
#include "SussInputProvider.h"
#include "SussGameplayTagInputProvider.generated.h"

/**
 * A base input provider class that changes input result depending on tags (not usable directly, see subclasses)
 */
UCLASS(Abstract)
class SUSS_API USussGameplayTagInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:
	/// Having any of these tags increases the score from this input 
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer PositiveScoreTags;

	/// The amount that's added to the score when a positive score tag is found
	UPROPERTY(EditDefaultsOnly)
	float PositiveScoreValue = 1.0;

	/// If true, scores will be altered by TagCount * Value. Otherwise 1+ tags will count as the same score. 
	UPROPERTY(EditDefaultsOnly)
	bool bTagCountAffectsScore = false;

	/// Having any of these tags reeduces the score from this input 
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer NegativeScoreTags;

	/// The amount that's subtracted to the score when a negative score tag is found
	UPROPERTY(EditDefaultsOnly)
	float NegativeScoreValue = 1.0;

	/// Whether to clamp the results to 0..1 
	UPROPERTY(EditDefaultsOnly)
	bool bClampResult = true;

	/// If you want to scale the result of multiple tags, increase this divisor to normalise the result
	UPROPERTY(EditDefaultsOnly)
	float Divisor = 1;

protected:
	float ScoreTagsOnActor(const AActor* Actor) const;

};

/**
 * An input provider class that changes input result depending on tags assigned to "Self", ie the owner of a brain
 */
UCLASS()
class SUSS_API USussGameplayTagSelfInputProvider : public USussGameplayTagInputProvider
{
	GENERATED_BODY()
public:
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};

/**
 * An input provider class that changes input result depending on tags assigned to the Target in a context
 */
UCLASS()
class SUSS_API USussGameplayTagTargetInputProvider : public USussGameplayTagInputProvider
{
	GENERATED_BODY()
public:
	virtual float Evaluate_Implementation(const class USussBrainComponent* Brain, const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};
