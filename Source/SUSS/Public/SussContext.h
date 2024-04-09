// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SussContext.generated.h"

typedef TVariant<FName,int,float,double> TSussCustomContextValue;
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
	/// Location which could vary per context (use determined by input provider)
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;
	/// Rotation which could vary per context (use determined by input provider)
	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation = FRotator::ZeroRotator;
	/// A gameplay tag which could represent any kind of identifying information (use determined by query & input provider)
	/// This can be useful for all kinds of things where a set of "something" needs to be queried & identified. For example,
	/// "get the set of attacks for this creature" could produce 1 context per attack tag for consideration
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;

	/// Any other custom context value you'd like to use (C++ only)
	TSussCustomContextValue Custom;

	bool operator==(const FSussContext& Other) const
	{
		return ControlledActor == Other.ControlledActor &&
			Target == Other.Target &&
			Location.Equals(Other.Location) &&
			Rotation.Equals(Other.Rotation);
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
		if (Custom.IsType<FName>())
		{
			// name is the default so we can filter out unset values
			if (Custom.Get<FName>() != NAME_None)
			{
				Builder.Appendf(TEXT(" Custom: %s"), *Custom.Get<FName>().ToString());
			}
		}
		else if (Custom.IsType<float>())
		{
			Builder.Appendf(TEXT(" Custom: %f"), Custom.Get<float>());
		}
		else if (Custom.IsType<int>())
		{
			Builder.Appendf(TEXT(" Custom: %d"), Custom.Get<int>());
		}
		else if (Custom.IsType<double>())
		{
			Builder.Appendf(TEXT(" Custom: %f"), Custom.Get<double>());
		}

		Builder.Append(" }");
		return Builder.ToString();
		
	}
};
