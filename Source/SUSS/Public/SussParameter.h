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
	/// The parameter is a vector
	Vector,
	/// The parameter is a boolean
	Bool,
	/// The parameter is a literal Name
	Name,
	/// The parameter is a literal gameplay tag
	Tag,
	/// The parameter is a literal gameplay tag container
	TagContainer,
	/// The parameter is provided automatically by either an Input Provider (floats only) or a Parameter Provider (all types)
	AutoParameter

};

USTRUCT(BlueprintType)
struct FSussParameter
{
	GENERATED_BODY()

public:
	/// The type of the parameter
	UPROPERTY(EditDefaultsOnly)
	ESussParamType Type = ESussParamType::Float;

	// We *could* save space by using a TVariant here, but that wouldn't be editable in the UI, so we live with the inefficiency
	
	/// Literal float value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Float", EditConditionHides))
	float FloatValue = 0;

	/// Literal vector value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Vector", EditConditionHides))
	FVector VectorValue = FVector::ZeroVector;

	/// Literal integer value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Int", EditConditionHides))
	int IntValue = 0;

	/// Literal boolean value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Bool", EditConditionHides))
	bool BoolValue = false;

	/// Literal name value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Name", EditConditionHides))
	FName NameValue = NAME_None;

	/// Literal tag value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::Tag", EditConditionHides))
	FGameplayTag Tag;

	/// Literal tag value of the parameter
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="Type==ESussParamType::TagContainer", EditConditionHides))
	FGameplayTagContainer TagContainer;
	

	/// Tag identifying the input or parameter provider we should use to populate into this parameter. 
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Suss.Input,Suss.Param", EditCondition="Type==ESussParamType::AutoParameter", EditConditionHides))
	FGameplayTag InputOrParameterTag;

	static FSussParameter ZeroLiteral;
	static FSussParameter OneLiteral;

	FSussParameter() {}
	FSussParameter(float Val) : Type(ESussParamType::Float), FloatValue(Val) {}
	FSussParameter(const FVector& Val) : Type(ESussParamType::Vector), VectorValue(Val) {}
	FSussParameter(int Val) : Type(ESussParamType::Int), IntValue(Val) {}
	explicit FSussParameter(bool Val) : Type(ESussParamType::Bool), BoolValue(Val) {}
	FSussParameter(FName Val) : Type(ESussParamType::Name), NameValue(Val) {}
	FSussParameter(FGameplayTag Val) : Type(ESussParamType::Tag), Tag(Val) {}
	FSussParameter(const FGameplayTagContainer& Val) : Type(ESussParamType::TagContainer), TagContainer(Val) {}

	friend bool operator==(const FSussParameter& Lhs, const FSussParameter& Rhs)
	{
		if (Lhs.Type != Rhs.Type)
			return false;

		switch (Lhs.Type)
		{
		case ESussParamType::Float:
			return FMath::IsNearlyEqual(Lhs.FloatValue, Rhs.FloatValue);
		case ESussParamType::Vector:
			return Lhs.VectorValue.Equals(Rhs.VectorValue);
		case ESussParamType::Int:
			return Lhs.IntValue == Rhs.IntValue;
		case ESussParamType::Name:
			return Lhs.NameValue == Rhs.NameValue;
		case ESussParamType::Tag:
			return Lhs.Tag == Rhs.Tag;
		case ESussParamType::TagContainer:
			return Lhs.TagContainer == Rhs.TagContainer;
		case ESussParamType::AutoParameter:
			return Lhs.InputOrParameterTag == Rhs.InputOrParameterTag;
		case ESussParamType::Bool:
			return Lhs.BoolValue == Rhs.BoolValue;
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
		case ESussParamType::Vector:
			Hash = HashCombine(Hash, GetTypeHash(Arg.VectorValue));
			break;
		case ESussParamType::Int:
			Hash = HashCombine(Hash, GetTypeHash(Arg.IntValue));
			break;
		case ESussParamType::Tag:
			Hash = HashCombine(Hash, GetTypeHash(Arg.Tag));
			break;
		case ESussParamType::TagContainer:
			{
				TArray<FGameplayTag> Tags;
				Arg.TagContainer.GetGameplayTagArray(Tags);
				for (auto& T : Tags)
				{
					Hash = HashCombine(Hash, GetTypeHash(T));
				}
			}
			break;
		case ESussParamType::AutoParameter:
			Hash = HashCombine(Hash, GetTypeHash(Arg.InputOrParameterTag));
			break;
		case ESussParamType::Bool:
			Hash = HashCombine(Hash, GetTypeHash(Arg.BoolValue));
			break;
		case ESussParamType::Name:
			break;
		};
		return Hash;
	}
};
