
#include "SussWorldSubsystem.h"

#include "SussBrainComponent.h"
#include "SussCommon.h"
#include "SussSettings.h"

USussWorldSubsystem::USussWorldSubsystem()
{
	if (const auto Settings = GetDefault<USussSettings>())
	{
		CachedFrameTimeBudgetMs = Settings->BrainUpdateFrameTimeBudgetMilliseconds;
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to load USussSettings, using hardcoded defaults"))
		CachedFrameTimeBudgetMs = 0.5f;
	}
}

bool USussWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void USussWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

TStatId USussWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USussWorldSubsystem, STATGROUP_Tickables);
}

void USussWorldSubsystem::Tick(float DeltaTime)
{
	UpdateBrains();
}

void USussWorldSubsystem::QueueBrainUpdate(USussBrainComponent* Brain)
{
	BrainsToUpdate.Enqueue(Brain);
}


void USussWorldSubsystem::UpdateBrains()
{
	const double StartTime = GetWorld()->GetTimeSeconds();
	while (!BrainsToUpdate.IsEmpty())
	{
		TWeakObjectPtr<USussBrainComponent> Brain;
		BrainsToUpdate.Dequeue(Brain);
		
		if (Brain.IsValid() && Brain->NeedsUpdate())
		{
			Brain->Update();
		}
		// Time limit
		if (GetWorld()->GetTimeSeconds() - StartTime >= CachedFrameTimeBudgetMs)
			break;
	}
}