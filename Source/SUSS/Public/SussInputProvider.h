// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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
class SUSS_API USussInputProvider : public UPrimaryDataAsset
{
	GENERATED_BODY()
protected:
	/// The tag which identifies the input which this provider is supplying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag;
	
public:

	USussInputProvider() {}
	
	const FGameplayTag& GetInputTag() const { return InputTag; }

	
	/// Evaluate the input given a context
	/// Also used to resolve parameters to queries and other inputs, in which case context is solely the Self reference
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)	
	float Evaluate(const FSussContext& Context, const TMap<FName, FSussParameter>& Parameters) const;
};
