// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SussParameter.h"

#include "SussParameterProvider.generated.h"

/**
 * A parameter provider is similar to an Input Provider, in that it provides a single value based on a context,
 * which are generated from Queries. The difference is that while Input Providers must only supply float values,
 * because they are fed into the scoring system, a Parameter Provider can provide any value which can be embedded
 * in a FSussParameter.
 *
 * These are mostly used to provide additional information to any other class which takes parameters as input,
 * such as input and query providers. Parameter providers provide an alternative to supplying literal parameters to
 * these classes (although note you can also use Input Providers to do this, so long as you only need float values)
 */
UCLASS()
class SUSS_API USussParameterProvider : public UPrimaryDataAsset
{
	GENERATED_BODY()
protected:
	/// The tag which identifies the parameter which this provider is supplying. Must be a subtag of "Suss.Param"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Suss.Param"))
	FGameplayTag ParameterTag;
	
public:

	USussParameterProvider() {}
	
	virtual FGameplayTag GetParameterTag() const { return ParameterTag; }

	
	/// Evaluate the parameter provider given a context
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)	
	FSussParameter Evaluate(const class USussBrainComponent* Brain, const FSussContext& Context, const TMap<FName, FSussParameter>& Parameters) const;
};
