// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SussContext.generated.h"


/// Base struct which can be used to store groups of related values in a context.
/// Normally, every value in a context multiplies the possibility space by all the others. But if you want, say 3 related
/// values that are always together in the context, you can put them in a subclass of this struct.
/// This is only really usable if you have access to C++; you can't define Blueprint subclasses and expect this to be useful.
/// You can, however, create C++ subclasses and some helper functions to make those available to Blueprints.
USTRUCT(BlueprintType)
struct FSussContextValueStructBase
{
	GENERATED_BODY()
	
	FSussContextValueStructBase() {}
	
};

/// The value type held by a context value
UENUM(BlueprintType)
enum class ESussContextValueType : uint8
{
	Actor,
	Vector,
	Rotator,
	Tag,
	Name,
	Float,
	Int,
	/// A subclass of FSussContextValueStructBase
	Struct,

	NONE
};

/// All the types that can be held in a named value on a context, should match ESussContextValueType
typedef TVariant<
	TWeakObjectPtr<AActor>,
	FVector,
	FRotator,
	FGameplayTag,
	FName,
	float,
	int,
	TSharedPtr<FSussContextValueStructBase>
> TSussContextValueVariant;

/// A flexibly typed value which can be present in a context so that inputs / autoparameters can use them
struct FSussContextValue
{
	ESussContextValueType Type = ESussContextValueType::NONE;
	TSussContextValueVariant Value;

	FSussContextValue(AActor* Actor) : Type(ESussContextValueType::Actor)
	{
		Value.Set<TWeakObjectPtr<AActor>>(MakeWeakObjectPtr(Actor));
	}
	FSussContextValue(const FVector& Vector) : Type(ESussContextValueType::Vector)
	{
		Value.Set<FVector>(Vector);
	}
	FSussContextValue(const FRotator& Rotator) : Type(ESussContextValueType::Rotator)
	{
		Value.Set<FRotator>(Rotator);
	}
	FSussContextValue(const FGameplayTag& Tag) : Type(ESussContextValueType::Tag)
	{
		Value.Set<FGameplayTag>(Tag);
	}
	FSussContextValue(const FName& Name) : Type(ESussContextValueType::Name)
	{
		Value.Set<FName>(Name);
	}
	FSussContextValue(float V) : Type(ESussContextValueType::Float)
	{
		Value.Set<float>(V);
	}
	FSussContextValue(int V) : Type(ESussContextValueType::Int)
	{
		Value.Set<int>(V);
	}
	FSussContextValue(const TSharedPtr<FSussContextValueStructBase>& V) : Type(ESussContextValueType::Struct)
	{
		Value.Set<TSharedPtr<FSussContextValueStructBase>>(V);
	}
	

	FString ToString() const
	{
		switch (Type)
		{
		case ESussContextValueType::Actor:
			return Value.Get<TWeakObjectPtr<AActor>>()->GetActorNameOrLabel();
		case ESussContextValueType::Vector:
			return Value.Get<FVector>().ToCompactString();
		case ESussContextValueType::Rotator:
			return Value.Get<FRotator>().ToCompactString();
		case ESussContextValueType::Tag:
			return Value.Get<FGameplayTag>().ToString();
		case ESussContextValueType::Name:
			return Value.Get<FName>().ToString();
		case ESussContextValueType::Float:
			return FString::SanitizeFloat(Value.Get<float>());
		case ESussContextValueType::Int:
			return FString::FromInt(Value.Get<int>());
		case ESussContextValueType::Struct:
			{
				const auto S = Value.Get<TSharedPtr<FSussContextValueStructBase>>();
				if (S.IsValid())
				{
					return S->StaticStruct()->GetName();
				}
				return "";
			}
		default:
		case ESussContextValueType::NONE:
			return TEXT("NONE");
		}
	}

	friend bool operator==(const FSussContextValue& Lhs, const FSussContextValue& Rhs)
	{
		if (Lhs.Type != Rhs.Type)
			return false;

		switch (Lhs.Type)
		{
		case ESussContextValueType::Actor:
			return Lhs.Value.Get<TWeakObjectPtr<AActor>>() == Rhs.Value.Get<TWeakObjectPtr<AActor>>();
		case ESussContextValueType::Vector:
			return Lhs.Value.Get<FVector>() == Rhs.Value.Get<FVector>();
		case ESussContextValueType::Rotator:
			return Lhs.Value.Get<FRotator>() == Rhs.Value.Get<FRotator>();
		case ESussContextValueType::Tag:
			return Lhs.Value.Get<FGameplayTag>() == Rhs.Value.Get<FGameplayTag>();
		case ESussContextValueType::Name:
			return Lhs.Value.Get<FName>() == Rhs.Value.Get<FName>();
		case ESussContextValueType::Float:
			return Lhs.Value.Get<float>() == Rhs.Value.Get<float>();
		case ESussContextValueType::Int:
			return Lhs.Value.Get<int>() == Rhs.Value.Get<int>();
		case ESussContextValueType::NONE:
			// Nones are equal
			return true;
		case ESussContextValueType::Struct:
			// Can't do this
			return false;
		}

		return false;
	}

	friend bool operator!=(const FSussContextValue& Lhs, const FSussContextValue& RHS)
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

	/// Named values of context for any other purpose
	TMap<FName, FSussContextValue> NamedValues;

	bool operator==(const FSussContext& Other) const
	{
		if (ControlledActor != Other.ControlledActor ||
			Target != Other.Target ||
			!Location.Equals(Other.Location) ||
			NamedValues.Num() != Other.NamedValues.Num())
		{
			return false;
		}
		
		for (auto& Pair : NamedValues)
		{
			if (const auto pOtherCustom = Other.NamedValues.Find(Pair.Key))
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

		for (const auto& Custom : NamedValues)
		{
			Builder.Appendf(TEXT(" %s: %s"), *Custom.Key.ToString(), *Custom.Value.ToString());
		}

		Builder.Append(" }");
		return Builder.ToString();
		
	}
};
