// 

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SussEQSWorldSubsystem.generated.h"

/**
 * This class is here to help with EQS support
 */
UCLASS()
class SUSS_API USussEQSWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	// This is a temporary map entirely for EQS target contexts. We need to know for a given query owner, what is the
	// current target being considered as context (for correlated queries). Since UEnvQueryContext has no information other
	// than the owner of the query and the query instance, and we don't want to force people to add a "target" property
	// to their actors (it wouldn't even be valid because you can consider multiple targets, which is what Suss contexts do),
	// this is the best way to do it. An EQS query adds an entry to this map with a PointerHash of the owner so
	// USussEnvQueryContext_Target can extract it
	UPROPERTY()
	TMap<uint32, AActor*> TargetContextMap;

	static uint32 GetTargetKey(const AActor* Owner) { return PointerHash(Owner); }

public:

	/// Associate a target with the owner of a query so it can be retrieved by USussEnvQueryContext_Target
	void SetTargetInfo(const AActor* Owner, AActor* Target);
	/// Clear the association between the owner of a query and a target
	void ClearTargetInfo(const AActor* Owner);
	/// Retrieve the current target context for a given query owner 
	AActor* GetTargetInfo(AActor* Owner);
};
