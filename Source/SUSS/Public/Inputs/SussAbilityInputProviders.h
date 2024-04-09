#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SussInputProvider.h"
#include "SussAbilityInputProviders.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputCanActivateAbility);

/**
 * Gets a value of 1 if an ability can be activated, and 0 if it cannot be activated.
 */
UCLASS()
class SUSS_API USussCanActivateAbilityInputProvider : public USussInputProvider
{
	GENERATED_BODY()

public:

	USussCanActivateAbilityInputProvider();
	
	virtual float Evaluate_Implementation(const USussBrainComponent* Brain,
		const FSussContext& Context,
		const TMap<FName, FSussParameter>& Parameters) const override;
};