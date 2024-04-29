// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EQSTestingPawn.h"
#include "SussEQSTestingPawn.generated.h"

UCLASS()
class SUSS_API ASussEQSTestingPawn : public AEQSTestingPawn
{
	GENERATED_BODY()

public:

	/// If you need a value that behaves like a context target, set it here
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor;

};
