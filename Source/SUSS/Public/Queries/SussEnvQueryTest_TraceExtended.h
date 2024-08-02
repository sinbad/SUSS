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

	DECLARE_DELEGATE_RetVal_SevenParams(bool, FDoTraceSignature, const FVector&, const FVector&, AActor*, UWorld*, enum ECollisionChannel, const FCollisionQueryParams&, const FVector&);
	bool DoLineTraceTo(const FVector& ItemPos,
	                          const FVector& ContextPos,
	                          AActor* ItemActor,
	                          UWorld* World,
	                          ECollisionChannel Channel,
	                          const FCollisionQueryParams& Params,
	                          const FVector& Extent);
	bool DoLineTraceFrom(const FVector& ItemPos,
	                     const FVector& ContextPos,
	                     AActor* ItemActor,
	                     UWorld* World,
	                     ECollisionChannel Channel,
	                     const FCollisionQueryParams& Params,
	                     const FVector& Extent);
	bool DoBoxTraceTo(const FVector& ItemPos,
	                  const FVector& ContextPos,
	                  AActor* ItemActor,
	                  UWorld* World,
	                  ECollisionChannel Channel,
	                  const FCollisionQueryParams& Params,
	                  const FVector& Extent);
	bool DoBoxTraceFrom(const FVector& ItemPos,
	                    const FVector& ContextPos,
	                    AActor* ItemActor,
	                    UWorld* World,
	                    ECollisionChannel Channel,
	                    const FCollisionQueryParams& Params,
	                    const FVector& Extent);
	bool DoSphereTraceTo(const FVector& ItemPos,
	                     const FVector& ContextPos,
	                     AActor* ItemActor,
	                     UWorld* World,
	                     ECollisionChannel Channel,
	                     const FCollisionQueryParams& Params,
	                     const FVector& Extent);
	bool DoSphereTraceFrom(const FVector& ItemPos,
	                       const FVector& ContextPos,
	                       AActor* ItemActor,
	                       UWorld* World,
	                       ECollisionChannel Channel,
	                       const FCollisionQueryParams& Params,
	                       const FVector& Extent);
	bool DoCapsuleTraceTo(const FVector& ItemPos,
	                      const FVector& ContextPos,
	                      AActor* ItemActor,
	                      UWorld* World,
	                      ECollisionChannel Channel,
	                      const FCollisionQueryParams& Params,
	                      const FVector& Extent);
	bool DoCapsuleTraceFrom(const FVector& ItemPos,
	                        const FVector& ContextPos,
	                        AActor* ItemActor,
	                        UWorld* World,
	                        ECollisionChannel Channel,
	                        const FCollisionQueryParams& Params,
	                        const FVector& Extent);

public:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
