// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SussContext.generated.h"

typedef TVariant<int,float,FVector> TSussContextValue;
/**
 * This object provides all the context required for many other SUSS classes to make their decisions and execute actions.
 * In the simplest case, there is only one context in which an action is evaluated, e.g. if an AI is considering what to
 * do about it's own health, for example. There is only one value of health, the AI's own, so one context.
 *
 * In more complicated cases, the action might be "Attack Enemy". In that case, there could be more than one enemy that
 * the AI could attack as part of that action, so there will be at least one context per enemy, with different values of
 * "Target". Now imagine that this is a ranged character attacking, and for each of those potential targets, there are
 * multiple vantage points to which the AI might want to move to take the shot. So now we have a multiplying effect on
 * the contexts that need to be evaluated to pick the highest scoring action; Target A from Location 1,
 * Target A from Location 2, Target B from Location 3, Target B from Location 4, and so on.
 */
USTRUCT(BlueprintType)
struct FSussContext
{
	GENERATED_BODY()
public:
	/// Actor which represents the pawn with the brain 
	UPROPERTY(BlueprintReadOnly)
	AActor* Self;
	/// Actor which represents the target in this context
	/// When multiple targets are possible, there are multiple contexts.
	UPROPERTY(BlueprintReadWrite)
	AActor* Target;

	/// Location which could vary per context (use determined by input provider)
	UPROPERTY(BlueprintReadWrite)
	FVector Location;
	/// Rotation which could vary per context (use determined by input provider)
	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;

	/// Map of other values that can be used by the input provider to add more parameterised contexts
	/// This is for if you had some variations of your own you wanted to add. Unfortunately C++ only.
	TMap<FName, TSussContextValue> OtherValues;

	
};
