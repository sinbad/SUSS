// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Templates/TypeHash.h"
#include "UObject/Object.h"
#include "SussParameter.generated.h"

UENUM(BlueprintType)
enum class ESussParamType : uint8
{
	/// The parameter is a literal float
	Float,
	/// The parameter is a literal int
	Int,
	/// The parameter is a literal Name
	Name,
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

	/// Literal integer value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Int", EditConditionHides))
	int IntValue = 0;

	/// Literal name value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Name", EditConditionHides))
	FName NameValue = NAME_None;

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

	FSussParameter() {}
	FSussParameter(float Val) : Type(ESussParamType::Float), FloatValue(Val) {}
	FSussParameter(int Val) : Type(ESussParamType::Int), IntValue(Val) {}
	FSussParameter(FName Val) : Type(ESussParamType::Name), NameValue(Val) {}
	FSussParameter(FGameplayTag Val) : Type(ESussParamType::Tag), Tag(Val) {}

	friend bool operator==(const FSussParameter& Lhs, const FSussParameter& Rhs)
	{
		if (Lhs.Type != Rhs.Type)
			return false;

		switch (Lhs.Type)
		{
		case ESussParamType::Float:
			return FMath::IsNearlyEqual(Lhs.FloatValue, Rhs.FloatValue);
		case ESussParamType::Int:
			return Lhs.IntValue == Rhs.IntValue;
		case ESussParamType::Name:
			return Lhs.NameValue == Rhs.NameValue;
		case ESussParamType::Tag:
			return Lhs.Tag == Rhs.Tag;
		case ESussParamType::Input:
			return Lhs.InputTag == Rhs.InputTag;
		}


		return false;
	}

	friend bool operator!=(const FSussParameter& Lhs, const FSussParameter& Rhs)
	{
		return !(Lhs == Rhs);
	}

	friend uint32 GetTypeHash(const FSussParameter& Arg)
	{
		uint32 Hash = GetTypeHash(Arg.Type);

		switch (Arg.Type)
		{
		case ESussParamType::Float:
			Hash = HashCombine(Hash, GetTypeHash(Arg.FloatValue));
			break;
		case ESussParamType::Int:
			Hash = HashCombine(Hash, GetTypeHash(Arg.IntValue));
			break;
		case ESussParamType::Tag:
			Hash = HashCombine(Hash, GetTypeHash(Arg.Tag));
			break;
		case ESussParamType::Input:
			Hash = HashCombine(Hash, GetTypeHash(Arg.InputTag));
			break;
		};
		return Hash;
	}
};
