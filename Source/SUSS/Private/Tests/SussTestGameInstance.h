// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SussTestGameInstance.generated.h"

/**
 * 
 */
UCLASS(transient)
class SUSS_API USussTestGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	void TestInit(UWorld* InWorld);
};
