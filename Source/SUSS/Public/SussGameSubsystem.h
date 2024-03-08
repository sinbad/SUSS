// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussInputProvider.h"
#include "SussQueryProvider.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SussGameSubsystem.generated.h"

/**
 * Global subsystem that's used to register providers that should remain regardless of World
 */
UCLASS()
class SUSS_API USussGameSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	TMap<FGameplayTag, USussInputProvider*> InputProviders;

	UPROPERTY()
	TMap<FGameplayTag, USussQueryProvider*> QueryProviders;

	UPROPERTY()
	USussInputProvider* DefaultInputProvider;

	TSet<FName> MissingTagsAlreadyWarnedAbout;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/// Register an input provider by class
	UFUNCTION(BlueprintCallable)
	void RegisterInputProvider(TSubclassOf<USussInputProvider> ProviderClass);

	USussInputProvider* GetInputProvider(const FGameplayTag& Tag);

	/// Register an input provider by class
	UFUNCTION(BlueprintCallable)
	void RegisterQueryProvider(TSubclassOf<USussQueryProvider> ProviderClass);

	USussQueryProvider* GetQueryProvider(const FGameplayTag& Tag);

	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual void Tick(float DeltaTime) override;
};

inline USussGameSubsystem* GetSUSS(UWorld* WorldContext)
{
	if (IsValid(WorldContext))
	{
		auto GI = WorldContext->GetGameInstance();
		if (IsValid(GI))
			return GI->GetSubsystem<USussGameSubsystem>();		
	}
		
	return nullptr;
}
