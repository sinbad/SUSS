// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussInputProvider.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SussEngineSubsystem.generated.h"

/**
 * Global subsystem that's mainly used to register providers that should remain regardless of Game Instances or World
 */
UCLASS()
class SUSS_API USussEngineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGameplayTag, USussInputProvider*> InputProviders;

	UPROPERTY()
	USussInputProvider* DefaultInputProvider;

	TSet<FName> MissingTagsAlreadyWarnedAbout;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/// Register an input provider by class
	UFUNCTION(BlueprintCallable)
	void RegisterInputProvider(TSubclassOf<USussInputProvider> ProviderClass);

	USussInputProvider* GetInputProvider(const FGameplayTag& Tag);
};

inline USussEngineSubsystem* GetSUSS(UWorld* WorldContext)
{
	if (IsValid(WorldContext) && WorldContext->IsGameWorld())
	{
		auto GI = WorldContext->GetGameInstance();
		if (IsValid(GI))
			return GI->GetSubsystem<USussEngineSubsystem>();		
	}
		
	return nullptr;
}
