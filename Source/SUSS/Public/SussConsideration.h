// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussCommon.h"
#include "SussParameter.h"
#include "UObject/Object.h"
#include "SussConsideration.generated.h"


/**
 * A consideration is a scoring function which is assigned to an action, returning a normalised utility value (0..1).
 * The function comprises:
 *
 * - An Input (from a list of possible inputs)
 * - Input bookends (which normalise the input)
 * - A Curve which converts the input to the score. This curve can be a pre-defined curve shape plus parameters, or a custom curve

 */
USTRUCT()
struct SUSS_API FSussConsideration
{
	GENERATED_BODY()

public:
	/// Optional description, for documentation and debugging
	UPROPERTY(EditDefaultsOnly)
	FString Description;

	/// Tag identifying the input value which we want to pull into this consideration.
	/// Input tags must have the prefix "Suss.Input"
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Suss.Input"))
	FGameplayTag InputTag;

	/// Optional parameters to the evaluation - these will depend on the InputTag
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FSussParameter> Parameters;

	/// Min value of interest of the input, which can be used to re-normalise the range.
	UPROPERTY(EditDefaultsOnly)
	FSussParameter BookendMin = FSussParameter::ZeroLiteral;

	/// Max value of interest of the input, which can be used to re-normalise the range.
	UPROPERTY(EditDefaultsOnly)
	FSussParameter BookendMax = FSussParameter::OneLiteral;

	UPROPERTY(EditDefaultsOnly)
	ESussCurveType CurveType = ESussCurveType::Linear;

	/// If curve is not custom, the parameters which change how the curve behaves (m,k,b,c)
	/// m = slope (step/linear/quadratic), 
	/// k = exponent (quadratic) or vertical change (step/logistic)
	/// b = y-shift
	/// c = x-shift
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="CurveType!=ESussCurveType::Custom", EditConditionHides))
	FVector4f CurveParams = FVector4f(1,1,0, 0); // default x=y linear

	/// If curve is custom, the actual curve to use.
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="CurveType==ESussCurveType::Custom", EditConditionHides))
	UCurveFloat* CustomCurve = nullptr;

	float EvaluateCurve(float Input) const;
};
