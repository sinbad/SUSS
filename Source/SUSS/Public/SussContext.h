// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SussContext.generated.h"

/**
 * This object provides all the context required for many other SUSS classes to make their decisions and execute actions.
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
	UPROPERTY(BlueprintReadOnly)
	AActor* Target;
	
	
};
