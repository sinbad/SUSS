#pragma once
#include "SussTestGameInstance.h"

#if WITH_AUTOMATION_TESTS
#include "EngineUtils.h"


// From https://minifloppy.it/posts/2024/automated-testing-specs-ue5/
class FSussTestWorldFixture
{
protected:
	/// Needed to assign a unique world name; atomic in case parallel test runner
	inline static std::atomic<uint32> WorldCounter = 0;
public:
	explicit FSussTestWorldFixture(const FURL& URL = FURL())
	{
		if (GEngine != nullptr)
		{
			if (UWorld* World = UWorld::CreateWorld(EWorldType::Game,
													false,
													FName(FString::Printf(TEXT("TestWorld%d"), WorldCounter++)),
													GetTransientPackage()))
			{
				FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
				WorldContext.SetCurrentWorld(World);

				// We need a test GI
				USussTestGameInstance* GI = NewObject<USussTestGameInstance>();
				GI->AddToRoot();
				GI->TestInit(World);
				WeakGI = MakeWeakObjectPtr(GI);
 
				World->InitializeActorsForPlay(URL);
				if (IsValid(World->GetWorldSettings()))
				{
					// Need to do this manually since world doesn't have a game mode
					World->GetWorldSettings()->NotifyBeginPlay();
					World->GetWorldSettings()->NotifyMatchStarted();
				}
				WeakWorld = MakeWeakObjectPtr(World);
				World->BeginPlay();
 
			}
		}
	}
 
	UWorld* GetWorld() const { return WeakWorld.Get(); }
 
	~FSussTestWorldFixture()
	{
		UWorld* World = WeakWorld.Get();
		if (World != nullptr && GEngine != nullptr)
		{
			World->BeginTearingDown();
 
			// Make sure to cleanup all actors immediately
			// DestroyWorld doesn't do this and instead waits for GC to clear everything up
			for (auto It = TActorIterator<AActor>(World); It; ++It)
			{
				It->Destroy();
			}
 
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UGameInstance* GI = WeakGI.Get();
		if (GI)
		{
			GI->Shutdown();
			GI->RemoveFromRoot();
			WeakGI.Reset();
		}
	}
 
private:
 
	TWeakObjectPtr<UWorld> WeakWorld;
	TWeakObjectPtr<USussTestGameInstance> WeakGI;
};

#endif