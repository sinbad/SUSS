
#include "SussTestGameInstance.h"

void USussTestGameInstance::TestInit(UWorld* InWorld)
{
	FWorldContext* TestWorldContext = GEngine->GetWorldContextFromWorld(InWorld);
	check(TestWorldContext);
	WorldContext = TestWorldContext;

	WorldContext->OwningGameInstance = this;
	InWorld->SetGameInstance(this);

	Init();
}
