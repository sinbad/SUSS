
#include "SussGameSubsystem.h"

#include "SussCommon.h"
#include "SussDummyProviders.h"
#include "SussSettings.h"
#include "Engine/ObjectLibrary.h"
#include "Queries/SussPerceptionQueries.h"

void USussGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DefaultInputProvider = NewObject<USussDummyInputProvider>();

	// Register any concrete query/input providers (rather than subclasses as dealt with later)
	RegisterQueryProviderClass(USussPerceptionKnownTargetsQueryProvider::StaticClass());
	RegisterQueryProviderClass(USussPerceptionKnownHostilesQueryProvider::StaticClass());
	RegisterQueryProviderClass(USussPerceptionKnownNonHostilesQueryProvider::StaticClass());

	// Set up libraries for input / query providers, for scanning for assets
	InputProviderLib = UObjectLibrary::CreateLibrary(USussInputProvider::StaticClass(), true, GIsEditor && !IsRunningCommandlet());
	if (GIsEditor)
	{
		InputProviderLib->bIncludeOnlyOnDiskAssets = false;
	}

	QueryProviderLib = UObjectLibrary::CreateLibrary(USussQueryProvider::StaticClass(), true, GIsEditor && !IsRunningCommandlet());
	if (GIsEditor)
	{
		QueryProviderLib->bIncludeOnlyOnDiskAssets = false;
	}

	// Auto set up based on settings
	if (auto Settings = GetDefault<USussSettings>())
	{
		// Explicitly registered classes
		for (auto& InputProv : Settings->InputProviders)
		{
			RegisterInputProviderClass(InputProv);
		}
		for (auto& QProv : Settings->QueryProviders)
		{
			RegisterQueryProviderClass(QProv);
		}

		// Asset-based providers

		TArray<FSoftObjectPath> InputProviderBPs;
		TArray<FSoftObjectPath> QueryProviderBPs;
		LoadClassesFromLibrary(Settings->InputProviderPaths, InputProviderLib, InputProviderBPs);
		LoadClassesFromLibrary(Settings->QueryProviderPaths, QueryProviderLib, QueryProviderBPs);

		for (auto& SoftRef : InputProviderBPs)
		{
			if (UClass* QClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				RegisterInputProviderClass(QClass);
			}
		}
		for (auto& SoftRef : QueryProviderBPs)
		{
			if (UClass* QClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				RegisterQueryProviderClass(QClass);
			}
		}
		
	}
}

void USussGameSubsystem::RegisterInputProvider(USussInputProvider* Provider)
{
	auto& Tag = Provider->GetInputTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = InputProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
				   Error,
				   TEXT("Unable to register Input Provider %s, tag %s already registered (%s)"),
				   *Provider->GetName(),
				   *Tag.GetTagName().ToString(),
				   *(*pExisting)->GetName())
		}
		else
		{
			InputProviders.Add(Tag, Provider);
			UE_LOG(LogSuss, Log, TEXT("Registered Input Provider %s for tag %s"), *Provider->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Input Provider %s, gameplay tag is invalid, did you remember to set it in your input provider?"), *Provider->GetName())
	}
}

void USussGameSubsystem::RegisterInputProviderClass(TSubclassOf<USussInputProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterInputProvider, invalid class (no CDO)"))
	}

	RegisterInputProvider(CDO);
}

USussInputProvider* USussGameSubsystem::GetInputProvider(const FGameplayTag& Tag)
{
	if (auto pProvider = InputProviders.Find(Tag))
	{
		return *pProvider;
	}

	if (!MissingTagsAlreadyWarnedAbout.Contains(Tag.GetTagName()))
	{
		UE_LOG(LogSuss, Warning, TEXT("Requested non-existent Input Provider for tag %s"), *Tag.GetTagName().ToString())
		MissingTagsAlreadyWarnedAbout.Add(Tag.GetTagName());
	}
	return DefaultInputProvider;
}

void USussGameSubsystem::RegisterQueryProvider(USussQueryProvider* Provider)
{
	auto& Tag = Provider->GetQueryTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = QueryProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
				   Error,
				   TEXT("Unable to register Query Provider %s, tag %s already registered (%s)"),
				   *Provider->GetName(),
				   *Tag.GetTagName().ToString(),
				   *(*pExisting)->GetName())
		}
		else
		{
			QueryProviders.Add(Tag, Provider);
			UE_LOG(LogSuss, Log, TEXT("Registered Query Provider %s for tag %s"), *Provider->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Query Provider %s, gameplay tag is invalid, did you remember to set it in your query provider?"), *Provider->GetName())
	}
}

void USussGameSubsystem::RegisterQueryProviderClass(TSubclassOf<USussQueryProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterQueryProvider, invalid class (no CDO)"))
	}
	RegisterQueryProvider(CDO);
	
}

void USussGameSubsystem::UnregisterQueryProvider(USussQueryProvider* Provider)
{
	auto& Tag = Provider->GetQueryTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = QueryProviders.Find(Tag))
		{
			if (*pExisting == Provider)
			{
				QueryProviders.Remove(Tag);
				return;
			}
		}
	}

	UE_LOG(LogSuss, Warning, TEXT("Possibly bad call to UnregisterQueryProvider, %s was not registered"), *Provider->GetName());

}

void USussGameSubsystem::UnregisterQueryProviderClass(TSubclassOf<USussQueryProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to UnregisterQueryProvider, invalid class %s (no CDO)"), *ProviderClass->GetName())
	}
	UnregisterQueryProvider(CDO);
}

USussQueryProvider* USussGameSubsystem::GetQueryProvider(const FGameplayTag& Tag)
{
	if (auto pProvider = QueryProviders.Find(Tag))
	{
		return *pProvider;
	}

	if (!MissingTagsAlreadyWarnedAbout.Contains(Tag.GetTagName()))
	{
		UE_LOG(LogSuss, Warning, TEXT("Requested non-existent Query Provider for tag %s"), *Tag.GetTagName().ToString())
		MissingTagsAlreadyWarnedAbout.Add(Tag.GetTagName());
	}
	return nullptr;
}

TStatId USussGameSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USussGameSubsystem, STATGROUP_SUSS);
}

UWorld* USussGameSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void USussGameSubsystem::Tick(float DeltaTime)
{
	// Tick the queries so they know when to invalidate results
	for (auto& Pair : QueryProviders)
	{
		Pair.Value->Tick(DeltaTime);
	}
}

void USussGameSubsystem::LoadClassesFromLibrary(const TArray<FDirectoryPath>& Paths,
	UObjectLibrary* ObjectLibrary,
	TArray<FSoftObjectPath>& OutSoftPaths)
{
	// LoadBlueprintAssetDataFromPaths requires FString array, LoadBlueprintAssetDataFromPath just makes one array per call so no better
	// I like using FDirectoryPath for the settings though since it enables browsing
	TArray<FString> StrPaths;
	for (auto& Dir : Paths)
	{
		StrPaths.Add(Dir.Path);
	}
	LoadClassesFromLibrary(StrPaths, ObjectLibrary, OutSoftPaths);
}

void USussGameSubsystem::LoadClassesFromLibrary(const TArray<FString>& Paths,
	UObjectLibrary* ObjectLibrary,
	TArray<FSoftObjectPath>& OutSoftPaths)
{
	ObjectLibrary->LoadBlueprintAssetDataFromPaths(Paths);
	ObjectLibrary->LoadAssetsFromAssetData();
	// Now they're all loaded, add them
	TArray<FAssetData> FoundAssets;
	ObjectLibrary->GetAssetDataList(FoundAssets);
	for (auto& Asset : FoundAssets)
	{
		// Need to resolve BP generated class
		const FString GeneratedClassTag = Asset.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
		if (GeneratedClassTag.IsEmpty())
		{
			UE_LOG(LogSuss, Warning, TEXT("Unable to find GeneratedClass value for asset %s"), *Asset.GetObjectPathString());
			continue;
		}
		FSoftObjectPath StringRef;
		StringRef.SetPath(FPackageName::ExportTextPathToObjectPath(GeneratedClassTag));
		OutSoftPaths.Add(StringRef);
	}
}

