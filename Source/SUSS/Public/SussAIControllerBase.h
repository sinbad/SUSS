// 

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SussAIControllerBase.generated.h"

/// Simple base AI controller which includes a SUSS brain component
/// You don't have to use this AIController, you can add USussBrainComponent to your own AI controllers if you want.
/// This is just for convenience.
UCLASS()
class SUSS_API ASussAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASussAIControllerBase();

	UFUNCTION(BlueprintPure)
	class USussBrainComponent* GetSussBrainComponent() const;

	UFUNCTION(BlueprintPure)
	const FSussBrainConfig& GetBrainConfig() const;

	UFUNCTION(BlueprintCallable)
	void SetBrainConfig(const FSussBrainConfig& NewConfig);

	UFUNCTION(BlueprintCallable)
	void SetBrainConfigFromAsset(USussBrainConfigAsset* Asset);

	/// Stop doing any current AI action
	UFUNCTION(BlueprintCallable)
	void StopCurrentAction();

};
