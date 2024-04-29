// 


#include "Queries/SussEnvQueryContext_Target.h"

#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Queries/SussEQSTestingPawn.h"
#include "Queries/SussEQSWorldSubsystem.h"

void USussEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                 FEnvQueryContextData& ContextData) const
{
	// SUSS allows multiple contexts, and a target per context. Since we have nowhere to store that on FEnvQueryInstance,
	// we use this subsystem hack to temporarily associate an owner with a target while we're running this query
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	auto EQSSub = QueryOwner->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
	AActor* Target = EQSSub->GetTargetInfo(QueryOwner);

#if WITH_EDITOR
	// This is just so that we can use it with the testing pawn
	if (!Target && QueryOwner->GetWorld()->IsEditorWorld())
	{
		if (auto TestingPawn = Cast<ASussEQSTestingPawn>(QueryOwner))
		{
			Target = TestingPawn->TargetActor;
		}
	}
#endif

	if (Target)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
	}
}
