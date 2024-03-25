#include "FGameplayDebuggerCategory_SUSS.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "SussBrainComponent.h"

FGameplayDebuggerCategory_SUSS::FGameplayDebuggerCategory_SUSS()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_SUSS::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	const APawn* Pawn = Cast<APawn>(DebugActor);
	const AAIController* Controller = Pawn ? Cast<AAIController>(Pawn->Controller) : nullptr;
	const USussBrainComponent* BrainComp = Controller ? Cast<USussBrainComponent>(GetValid(Controller->GetBrainComponent())) : nullptr;
	
	if (BrainComp)
	{
		DataPack.BrainDebugText = BrainComp->GetDebugInfoString();
		DataPack.BrainDebugLocations.Empty();
		BrainComp->DebugLocations(DataPack.BrainDebugLocations);
	}
}

void FGameplayDebuggerCategory_SUSS::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!DataPack.BrainDebugText.IsEmpty())
	{
		UWorld* World = CanvasContext.GetWorld();
		CanvasContext.Print(DataPack.BrainDebugText);

		for (auto& Loc : DataPack.BrainDebugLocations)
		{
			DrawDebugSphere(World, Loc, 30, 8, FColor::Cyan);
		}
	}
}
TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_SUSS::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_SUSS());
}

void FGameplayDebuggerCategory_SUSS::FRepData::Serialize(FArchive& Ar)
{
	Ar << BrainDebugText;
	Ar << BrainDebugLocations;
}