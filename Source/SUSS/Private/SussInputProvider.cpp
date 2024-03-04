// 


#include "SussInputProvider.h"

void USussInputProvider::GenerateContexts(AActor* Self, TArray<FSussContext>& OutputContexts) const
{
	// By default, generate a single context
	OutputContexts.Add(FSussContext {Self});
}

float USussInputProvider::Evaluate(const FSussContext& Context) const 
{
	return 0;
}
