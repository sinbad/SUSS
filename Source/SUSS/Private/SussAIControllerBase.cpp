
#include "SussAIControllerBase.h"

#include "SussBrainComponent.h"

ASussAIControllerBase::ASussAIControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	BrainComponent = CreateDefaultSubobject<USussBrainComponent>("SUSS Brain");
	AddInstanceComponent(BrainComponent);

	// We need StartLogic to be called for our brain to work, doing it on possess is easiest
	// Not sure why this isn't default, perhaps because you're supposed to activate it when players come near
	// However our brain component already accounts for distance & throttles when far away
	bStartAILogicOnPossess = true;
	
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
