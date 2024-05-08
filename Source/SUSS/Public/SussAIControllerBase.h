// 

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SussAIControllerBase.generated.h"

struct FSussBrainConfig;
/// Simple base AI controller which includes a SUSS brain component
/// You don't have to use this AIController, you can add USussBrainComponent to your own AI controllers if you want.
/// This is just for convenience.
UCLASS()
class SUSS_API ASussAIControllerBase : public AAIController
{
	GENERATED_BODY()
protected:
	/// If > 0 will lead the target when set to focus on an actor
	float LeadTargetProjectileVelocity = 0;

public:
	// Sets default values for this actor's properties
	ASussAIControllerBase(const FObjectInitializer& ObjectInitializer);

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

	
	/**
	 * When a focus actor is set, instead of focussing on it directly, focus on a point ahead of where it's moving
	 * to, in order to "lead the target" and shoot projectiles more accurately.
	 * @param ProjectileVelocity The velocity of projectiles, which will be used to calculate how much to
	 *   lead by (together with actor velocity)
	 */
	UFUNCTION(BlueprintCallable, Category="AI")
	void SetFocusLeadTarget(float ProjectileVelocity) { LeadTargetProjectileVelocity = ProjectileVelocity; }

	/// Stop leading focus targets and go back to focusing directly on them
	UFUNCTION(BlueprintCallable, Category="AI")
	void ClearFocusLeadTarget() { LeadTargetProjectileVelocity = 0; }

	// Overriden to support leading moving targets
	virtual FVector GetFocalPointOnActor(const AActor* Actor) const override;
};
