#include "FGameplayDebuggerCategory_SUSS.h"

#include "AIController.h"
#include "BrainComponent.h"

FGameplayDebuggerCategory_SUSS::FGameplayDebuggerCategory_SUSS()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_SUSS::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	const APawn* Pawn = Cast<APawn>(DebugActor);
	const AAIController* Controller = Pawn ? Cast<AAIController>(Pawn->Controller) : nullptr;
	const UBrainComponent* BrainComp = GetValid(Controller ? Controller->GetBrainComponent() : nullptr);
	
	if (BrainComp)
	{
		DataPack.BrainDebugText = BrainComp->GetDebugInfoString();
	}
}

void FGameplayDebuggerCategory_SUSS::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!DataPack.BrainDebugText.IsEmpty())
	{
		CanvasContext.Print(DataPack.BrainDebugText);
	}
}
TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_SUSS::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_SUSS());
}

void FGameplayDebuggerCategory_SUSS::FRepData::Serialize(FArchive& Ar)
{
	Ar << BrainDebugText;
}