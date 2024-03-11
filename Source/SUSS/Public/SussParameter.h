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
	Float,
	/// The parameter is a literal int
	Int,
	/// The parameter is a gameplay tag
	Tag,
	/// The parameter comes from an input with the current context (input must be able to extract ONE float value from the current context)
	Input

};

USTRUCT(BlueprintType)
struct FSussParameter
{
	GENERATED_BODY()

public:
	/// The type of the parameter
	UPROPERTY(EditDefaultsOnly)
	ESussParamType Type = ESussParamType::Float;

	/// Literal float value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Float", EditConditionHides))
	float FloatValue = 0;

	/// Literal value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Int", EditConditionHides))
	int IntValue = 0;

	/// Literal value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Tag", EditConditionHides))
	FGameplayTag Tag;

	/// Tag identifying the input value we should pull into this parameter. Can only reference "Self" ie the brain actor,
	/// because the value is used in other queries which have yet to provide any more information. Therefore the
	/// tags are limited to subtags of "Suss.Input.Self"
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Suss.Input.Self", EditCondition="Type==ESussParamType::Input", EditConditionHides))
	FGameplayTag InputTag;

	static FSussParameter ZeroLiteral;
	static FSussParameter OneLiteral;

	friend bool operator==(const FSussParameter& Lhs, const FSussParameter& Rhs)
	{
		if (Lhs.Type != Rhs.Type)
			return false;

		if (Lhs.Type == ESussParamType::Float)
			return FMath::IsNearlyEqual(Lhs.FloatValue, Rhs.FloatValue);

		if (Lhs.Type == ESussParamType::Int)
			return Lhs.IntValue == Rhs.IntValue;
		
		if (Lhs.Type == ESussParamType::Tag)
			return Lhs.Tag == Rhs.Tag;

		// for input, return the same tag
		return Lhs.InputTag == Rhs.InputTag;
		
	}

	friend bool operator!=(const FSussParameter& Lhs, const FSussParameter& Rhs)
	{
		return !(Lhs == Rhs);
	}
};
