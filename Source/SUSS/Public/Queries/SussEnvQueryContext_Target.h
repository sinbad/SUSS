// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "SussEnvQueryContext_Target.generated.h"

/**
 * EQS helper to expose the current target being considered as an EQS Context value
 */
UCLASS()
class SUSS_API USussEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
