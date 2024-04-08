#pragma once
#include "GameplayDebuggerCategory.h"

class FGameplayDebuggerCategory_SUSS : public FGameplayDebuggerCategory
{
protected:
	struct FRepData
	{
		// this is where we put all the data we collect
		FString BrainDebugText;
		TArray<FVector> BrainDebugLocations;
		TArray<FString> DetailTextLines;
		
		FRepData() 
		{
		}

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;
	bool bShowDetails = false;
	
	void ToggleDetailView();
	
public:
	FGameplayDebuggerCategory_SUSS();
	
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();
};
