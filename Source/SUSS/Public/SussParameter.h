// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SussParameter.generated.h"

UENUM(BlueprintType)
enum class ESussParamType : uint8
{
	/// The parameter is a literal float
	Literal,
	/// The parameter comes from an input with the current context (input must be able to extract ONE value from the current context)
	Input

	// TODO: perhaps support parsed expressions sometime
};

USTRUCT(BlueprintType)
struct FSussParameter
{
	GENERATED_BODY()

public:
	/// The type of the parameter
	UPROPERTY(EditDefaultsOnly)
	ESussParamType Type;

	/// Literal value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Literal", EditConditionHides))
	float Value;

	/// Tag identifying the input value we should pull into this parameter
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Suss.Input", EditCondition="Type==ESussParamType::Input", EditConditionHides))
	FGameplayTag InputTag;

	static FSussParameter ZeroLiteral;
	static FSussParameter OneLiteral;
	
};
