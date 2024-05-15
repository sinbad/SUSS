// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "SussEnvQueryTest_TraceExtended.generated.h"

/**
 * Enhanced version of UEnvQueryTest_Trace which allows you to start / end the trace at an offset from
 * the item / context. This can be very useful when you want a wide trace, but this would cause it to return
 * false positives if traced all the way to the item / context, because it could clip things on the other side.
 */
UCLASS()
class SUSS_API USussEnvQueryTest_TraceExtended : public UEnvQueryTest_Trace
{
	GENERATED_BODY()

	/// Offset along the line of the trace to start/end near the item
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	FAIDataProviderFloatValue ItemTraceOffset;

	/// Offset along the line of the trace to start/end near the item
	UPROPERTY(EditDefaultsOnly, Category=Trace, AdvancedDisplay)
	FAIDataProviderFloatValue ContextTraceOffset;

public:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
