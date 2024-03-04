// 

#pragma once

#include "CoreMinimal.h"
#include "SussInputProvider.h"
#include "UObject/Object.h"
#include "SussDummyProviders.generated.h"

/**
 * Dummy input provider for when invalid tag requested
 */
UCLASS()
class SUSS_API USussDummyInputProvider : public USussInputProvider
{
	GENERATED_BODY()
};
