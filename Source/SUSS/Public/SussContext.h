// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SussContext.generated.h"

/// The value type held by a context value
UENUM(BlueprintType)
enum class ESussNamedContextValueType : uint8
{
	Actor,
	Vector,
	Rotator,
	Tag,
	Name,
	Float,
	Int,

	NONE
};
typedef TVariant<TWeakObjectPtr<AActor>,FVector,FRotator, FGameplayTag,FName,int,float> TSussContextValueVariant;

struct FSussNamedContextValue
{
	ESussNamedContextValueType Type = ESussNamedContextValueType::NONE;
	TSussContextValueVariant Value;

	FSussNamedContextValue(AActor* Actor) : Type(ESussNamedContextValueType::Actor)
	{
		Value.Set<TWeakObjectPtr<AActor>>(MakeWeakObjectPtr(Actor));
	}
	FSussNamedContextValue(const FVector& Vector) : Type(ESussNamedContextValueType::Vector)
	{
		Value.Set<FVector>(Vector);
	}
	FSussNamedContextValue(const FRotator& Rotator) : Type(ESussNamedContextValueType::Rotator)
	{
		Value.Set<FRotator>(Rotator);
	}
	FSussNamedContextValue(const FGameplayTag& Tag) : Type(ESussNamedContextValueType::Tag)
	{
		Value.Set<FGameplayTag>(Tag);
	}
	FSussNamedContextValue(const FName& Name) : Type(ESussNamedContextValueType::Name)
	{
		Value.Set<FName>(Name);
	}
	FSussNamedContextValue(float V) : Type(ESussNamedContextValueType::Float)
	{
		Value.Set<float>(V);
	}
	FSussNamedContextValue(int V) : Type(ESussNamedContextValueType::Int)
	{
		Value.Set<int>(V);
	}

	FString ToString() const
	{
		switch (Type)
		{
		case ESussNamedContextValueType::Actor:
			return Value.Get<TWeakObjectPtr<AActor>>()->GetActorNameOrLabel();
		case ESussNamedContextValueType::Vector:
			return Value.Get<FVector>().ToCompactString();
		case ESussNamedContextValueType::Rotator:
			return Value.Get<FRotator>().ToCompactString();
		case ESussNamedContextValueType::Tag:
			return Value.Get<FGameplayTag>().ToString();
		case ESussNamedContextValueType::Name:
			return Value.Get<FName>().ToString();
		case ESussNamedContextValueType::Float:
			return FString::SanitizeFloat(Value.Get<float>());
		case ESussNamedContextValueType::Int:
			return FString::FromInt(Value.Get<int>());
		default:
		case ESussNamedContextValueType::NONE:
			return TEXT("NONE");
		}
	}

	friend bool operator==(const FSussNamedContextValue& Lhs, const FSussNamedContextValue& Rhs)
	{
		if (Lhs.Type != Rhs.Type)
			return false;

		switch (Lhs.Type)
		{
		case ESussNamedContextValueType::Actor:
			return Lhs.Value.Get<TWeakObjectPtr<AActor>>() == Rhs.Value.Get<TWeakObjectPtr<AActor>>();
		case ESussNamedContextValueType::Vector:
			return Lhs.Value.Get<FVector>() == Rhs.Value.Get<FVector>();
		case ESussNamedContextValueType::Rotator:
			return Lhs.Value.Get<FRotator>() == Rhs.Value.Get<FRotator>();
		case ESussNamedContextValueType::Tag:
			return Lhs.Value.Get<FGameplayTag>() == Rhs.Value.Get<FGameplayTag>();
		case ESussNamedContextValueType::Name:
			return Lhs.Value.Get<FName>() == Rhs.Value.Get<FName>();
		case ESussNamedContextValueType::Float:
			return Lhs.Value.Get<float>() == Rhs.Value.Get<float>();
		case ESussNamedContextValueType::Int:
			return Lhs.Value.Get<int>() == Rhs.Value.Get<int>();
		case ESussNamedContextValueType::NONE:
			// Nones are equal
			return true;
		}

		return false;
	}

	friend bool operator!=(const FSussNamedContextValue& Lhs, const FSussNamedContextValue& RHS)
	{
		return !(Lhs == RHS);
	}
};
/**
 * This object provides all the context required for many other SUSS classes to make their decisions and execute actions.
 * In the simplest case, there is only one context in which an action is evaluated, e.g. if an AI is considering what to
 * do about it's own health, for example. There is only one value of health, the AI's own, so one context.
 *
 * In more complicated cases, the action might be "Attack Enemy". In that case, there could be more than one enemy that
 * the AI could attack as part of that action, so there will be at least one context per enemy, with different values of
 * "Target". Now imagine that AI has multiple types of attack, each with its own characteristics. So now you have
 * a matrix of potential contexts: different targets, and different attack types, the resulting set of contexts being
 * potentially all combinations of those, each of which can be scored individually.
 */
USTRUCT(BlueprintType)
struct FSussContext
{
	GENERATED_BODY()
public:
	/// Actor which represents the pawn with the brain, this is always present
	UPROPERTY(BlueprintReadOnly)
	AActor* ControlledActor = nullptr;
	
	/// Actor which represents the target in this context, if applicable
	/// When multiple targets are possible, there are multiple contexts.
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Target;
	/// Location which could vary per context (use determined by query provider)
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;
	/// Rotation which could vary per context (use determined by query provider)
	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation = FRotator::ZeroRotator;
	/// A gameplay tag which could represent any kind of identifying information (use determined by query & input provider)
	/// This can be useful for all kinds of things where a set of "something" needs to be queried & identified. For example,
	/// "get the set of attacks for this creature" could produce 1 context per attack tag for consideration
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;

	/// Any other custom context values you'd like to use (C++ only)
	TMap<FName, FSussNamedContextValue> CustomValues;

	bool operator==(const FSussContext& Other) const
	{
		if (ControlledActor != Other.ControlledActor ||
			Target != Other.Target ||
			!Location.Equals(Other.Location) ||
			!Rotation.Equals(Other.Rotation) ||
			Tag != Other.Tag ||
			CustomValues.Num() != Other.CustomValues.Num())
		{
			return false;
		}
		
		for (auto& Pair : CustomValues)
		{
			if (const auto pOtherCustom = Other.CustomValues.Find(Pair.Key))
			{
				if (*pOtherCustom != Pair.Value)
				{
					return false;
				}
			}
		}

		return true;
					
	}

	FString ToString() const
	{
		TStringBuilder<256> Builder;
		Builder.Append("Context {");
		if (Target.IsValid())
		{
			Builder.Append(" Target: ");
			Builder.Append(Target->GetActorNameOrLabel());
		}
		if (!Location.IsNearlyZero())
		{
			Builder.Append(" Location: ");
			Builder.Append(Location.ToString());
		}
		if (!Rotation.IsNearlyZero())
		{
			Builder.Append(" Rotation: ");
			Builder.Append(Rotation.ToString());
		}
		if (Tag.IsValid())
		{
			Builder.Append(" Tag: ");
			Builder.Append(Tag.ToString());
		}

		for (const auto& Custom : CustomValues)
		{
			Builder.Appendf(TEXT(" %s: %s"), *Custom.Key.ToString(), *Custom.Value.ToString());
		}

		Builder.Append(" }");
		return Builder.ToString();
		
	}
};
