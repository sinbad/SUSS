// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SussAction.h"
#include "SussActionSetAsset.h"
#include "SussInputProvider.h"
#include "SussQueryProvider.h"
#include "Engine/ObjectLibrary.h"
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
	TMap<FGameplayTag, TSubclassOf<USussAction>> ActionClasses;

	UPROPERTY()
	TMap<FGameplayTag, USussInputProvider*> InputProviders;

	UPROPERTY()
	TMap<FGameplayTag, USussQueryProvider*> QueryProviders;

	UPROPERTY()
	USussInputProvider* DefaultInputProvider;

	UPROPERTY()
	UObjectLibrary* ActionClassLib;
	UPROPERTY()
	UObjectLibrary* InputProviderLib;
	UPROPERTY()
	UObjectLibrary* QueryProviderLib;

	TSet<FName> MissingTagsAlreadyWarnedAbout;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Register an action class
	UFUNCTION(BlueprintCallable)
	void RegisterActionClass(TSubclassOf<USussAction> ActionClass);

	/// Unregister an action class
	UFUNCTION(BlueprintCallable)
	void UnregisterActionClass(TSubclassOf<USussAction> ActionClass);

	TSubclassOf<USussAction> GetActionClass(FGameplayTag ActionTag);

	/// Register an input provider by class
	UFUNCTION(BlueprintCallable)
	void RegisterInputProviderClass(TSubclassOf<USussInputProvider> ProviderClass);
	/// Register an input provider instance
	UFUNCTION(BlueprintCallable)
	void RegisterInputProvider(USussInputProvider* Provider);

	UFUNCTION(BlueprintCallable)
	USussInputProvider* GetInputProvider(const FGameplayTag& Tag);

	/// Register a query provider by class
	UFUNCTION(BlueprintCallable)
	void RegisterQueryProviderClass(TSubclassOf<USussQueryProvider> ProviderClass);
	/// Register an query provider instance
	UFUNCTION(BlueprintCallable)
	void RegisterQueryProvider(USussQueryProvider* Provider);

	/// Unregister a query provider by class
	UFUNCTION(BlueprintCallable)
	void UnregisterQueryProviderClass(TSubclassOf<USussQueryProvider> ProviderClass);

	/// Unregister a query provider instance
	UFUNCTION(BlueprintCallable)
	void UnregisterQueryProvider(USussQueryProvider* Provider);

	UFUNCTION(BlueprintCallable)
	USussQueryProvider* GetQueryProvider(const FGameplayTag& Tag);

	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual void Tick(float DeltaTime) override;

protected:
	void LoadClassesFromLibrary(const TArray<FDirectoryPath>& Paths, UObjectLibrary* ObjectLibrary, TArray<FSoftObjectPath>& OutSoftPaths);
	void LoadClassesFromLibrary(const TArray<FString>& Paths, UObjectLibrary* ObjectLibrary, TArray<FSoftObjectPath>& OutSoftPaths);
	void RegisterNativeProviders();


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
