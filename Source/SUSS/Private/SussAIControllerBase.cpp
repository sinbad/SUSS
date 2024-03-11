
#include "SussAIControllerBase.h"

#include "SussBrainComponent.h"

ASussAIControllerBase::ASussAIControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	BrainComponent = CreateDefaultSubobject<USussBrainComponent>("SUSS Brain");
	
}

USussBrainComponent* ASussAIControllerBase::GetSussBrainComponent() const
{
	return Cast<USussBrainComponent>(BrainComponent);
}

const FSussBrainConfig& ASussAIControllerBase::GetBrainConfig() const
{
	if (auto B = GetSussBrainComponent())
	{
		return B->GetBrainConfig();
	}

	static FSussBrainConfig Dummy;
	return Dummy;
}

void ASussAIControllerBase::SetBrainConfig(const FSussBrainConfig& NewConfig)
{
	if (auto B = GetSussBrainComponent())
	{
		B->SetBrainConfig(NewConfig);
	}
}

void ASussAIControllerBase::SetBrainConfigFromAsset(USussBrainConfigAsset* Asset)
{
	if (auto B = GetSussBrainComponent())
	{
		B->SetBrainConfigFromAsset(Asset);
	}
}

void ASussAIControllerBase::StopCurrentAction()
{
	if (auto B = GetSussBrainComponent())
	{
		B->StopCurrentAction();
	}
}
