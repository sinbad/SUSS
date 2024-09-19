#pragma once
#include "CoreMinimal.h"
#include "SussTargetInterface.generated.h"

UINTERFACE(MinimalAPI)
class USussTargetInterface : public UInterface
{
	GENERATED_BODY()
	// This will always be empty!
};


/// Interface you should implement to provide more information to SUSS about an actor which is a target
/// of AI interest.
class SUSS_API ISussTargetInterface
{
	GENERATED_BODY()

public:
	/// Return an offset in local space from the actor's origin representing the focal point for
	/// an AI. Useful if your target's origin is at its feet but AIs should aim at its center of mass.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Suss|Target")
	FVector GetFocalPointLocalSpace() const;
	
};

