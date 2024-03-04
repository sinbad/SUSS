// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussContext.h"
#include "UObject/Object.h"
#include "SussInputProvider.generated.h"



/**
 * An input provider supplies an implementation for a input tag.
 * Every input provider must:
 * 1) Define a gameplay tag representing the input, which must begin with "Suss.Input".
 *	  You can do this either using UE_DEFINE_GAMEPLAY_TAG_STATIC or adding it to tag INI files
 * 2) Create a subclass of USussInputProvider which defines & implements this input
 * 3) Register this class with USussEngineSubsystem::RegisterInputProvider
 *
 * Inputs can be a single value, or they can be a collection of values, each of which will be considered as a separate
 * context for a potential action. For example, if you wanted an input which was the location of cover points,
 * your input could produce a selection of them, each of which would be run through the considerations for an action
 * such as "take cover", so the best candidate could be found.
 *
 * Input providers themselves are stateless, which is why only the class needs to be registered, SUSS uses the CDO.
 */
UCLASS(Blueprintable, Abstract)
class SUSS_API USussInputProvider : public UObject
{
	GENERATED_BODY()
protected:
	FGameplayTag GameplayTag;
public:
	const FGameplayTag& GetGameplayTag() const { return GameplayTag; }

	/// Generate a list of contexts based on what variants this input can provide
	void GenerateContexts(AActor* Self, TArray<FSussContext>& OutputContexts) const;

	/// Evaluate the input given a context
	float Evaluate(const FSussContext& Context) const;
};
