#include "SussBrainComponent.h"

#include "AIController.h"
#include "SussSettings.h"
#include "SussWorldSubsystem.h"


// Sets default values for this component's properties
USussBrainComponent::USussBrainComponent(): bQueuedForUpdate(false), TimeSinceLastUpdate(0), CachedUpdateRequestTime(1)
{
	// Brains tick in order to queue themselves for update regularly
	PrimaryComponentTick.bCanEverTick = true;

	if (auto Settings = GetDefault<USussSettings>())
	{
		CachedUpdateRequestTime = Settings->BrainUpdateRequestIntervalSeconds;
	}

}


// Called when the game starts
void USussBrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// No need to tick on non-server
	if (!GetOwner()->HasAuthority())
	{
		SetComponentTickEnabled(false);
	}

}


// Called every frame
void USussBrainComponent::TickComponent(float DeltaTime,
                                        ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->HasAuthority())
	{
		TimeSinceLastUpdate += DeltaTime;
		if (!bQueuedForUpdate && TimeSinceLastUpdate > CachedUpdateRequestTime)
		{
			if (auto SS = GetSussWorldSubsystem(GetWorld()))
			{
				SS->QueueBrainUpdate(this);
				bQueuedForUpdate = true;
			}
		}
	}
}

void USussBrainComponent::Update()
{
	if (!GetOwner()->HasAuthority())
		return;
	
	// TODO: update decision

	bQueuedForUpdate = false;
	TimeSinceLastUpdate = 0;
}

AAIController* USussBrainComponent::GetAIController() const
{
	if (const auto Cached = AiController.Get())
	{
		return Cached;
	}

	AAIController* Found = Cast<AAIController>(GetOwner());
	if (!Found)
	{
		if (const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			Found = Cast<AAIController>(Pawn->GetController());
		}
	}

	if (Found)
	{
		AiController = Found;
		return Found;
	}

	return nullptr;
	
}

