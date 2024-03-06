// 


#include "SussInputProvider.h"

void USussInputProvider::GenerateContexts_Implementation(AActor* SelfActor, TArray<FSussContext>& OutputContexts) const
{
	// By default, generate a single context
	OutputContexts.Add(FSussContext {SelfActor});
}

float USussInputProvider::Evaluate_Implementation(const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	return 0;
}
