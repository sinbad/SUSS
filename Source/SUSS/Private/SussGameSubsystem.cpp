
#include "SussGameSubsystem.h"

#include "SussCommon.h"
#include "SussDummyProviders.h"
#include "SussSettings.h"
#include "Engine/ObjectLibrary.h"
#include "..\Public\Inputs\SussPerceptionInputProviders.h"
#include "Actions/SussAbilityActions.h"
#include "Inputs/SussAbilityInputProviders.h"
#include "Inputs/SussBlackboardInputProviders.h"
#include "Inputs/SussBrainInfoInputProviders.h"
#include "Inputs/SussDistanceInputProviders.h"
#include "Queries/SussPerceptionQueries.h"

void USussGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DefaultInputProvider = NewObject<USussDummyInputProvider>();

	RegisterNativeProviders();

	// Set up libraries for actions / input / query providers, for scanning for assets
	ActionClassLib = UObjectLibrary::CreateLibrary(USussAction::StaticClass(), true, GIsEditor && !IsRunningCommandlet());
	if (GIsEditor)
	{
		ActionClassLib->bIncludeOnlyOnDiskAssets = false;
	}

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

	ParamProviderLib = UObjectLibrary::CreateLibrary(USussParameterProvider::StaticClass(), true, GIsEditor && !IsRunningCommandlet());
	if (GIsEditor)
	{
		ParamProviderLib->bIncludeOnlyOnDiskAssets = false;
	}

	// Auto set up based on settings
	if (auto Settings = GetDefault<USussSettings>())
	{
		// Explicitly registered classes
		for (auto& ActionClass : Settings->ActionClasses)
		{
			RegisterActionClass(ActionClass);
		}
		for (auto& InputProv : Settings->InputProviders)
		{
			RegisterInputProviderClass(InputProv);
		}
		for (auto& QProv : Settings->QueryProviders)
		{
			RegisterQueryProviderClass(QProv);
		}
		for (auto& PProv : Settings->ParameterProviders)
		{
			RegisterParameterProviderClass(PProv);
		}

		// Asset-based providers

		TArray<FSoftObjectPath> ActionClassBPs;
		TArray<FSoftObjectPath> InputProviderBPs;
		TArray<FSoftObjectPath> QueryProviderBPs;
		TArray<FSoftObjectPath> ParamProviderBPs;
		LoadClassesFromLibrary(Settings->ActionClassPaths, ActionClassLib, ActionClassBPs);
		LoadClassesFromLibrary(Settings->InputProviderPaths, InputProviderLib, InputProviderBPs);
		LoadClassesFromLibrary(Settings->QueryProviderPaths, QueryProviderLib, QueryProviderBPs);
		LoadClassesFromLibrary(Settings->ParameterProviderPaths, ParamProviderLib, ParamProviderBPs);

		for (auto& SoftRef : ActionClassBPs)
		{
			if (UClass* ActionClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				if (!ActionClass->HasAnyClassFlags(CLASS_Abstract))
				{
					RegisterActionClass(ActionClass);
				}
			}
		}
		for (auto& SoftRef : InputProviderBPs)
		{
			if (UClass* IpClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				if (!IpClass->HasAnyClassFlags(CLASS_Abstract))
				{
					RegisterInputProviderClass(IpClass);
				}
			}
		}
		for (auto& SoftRef : QueryProviderBPs)
		{
			if (UClass* QClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				if (!QClass->HasAnyClassFlags(CLASS_Abstract))
				{
					RegisterQueryProviderClass(QClass);
				}
			}
		}
		for (auto& SoftRef : ParamProviderBPs)
		{
			if (UClass* PClass = Cast<UClass>(SoftRef.ResolveObject()))
			{
				if (!PClass	->HasAnyClassFlags(CLASS_Abstract))
				{
					RegisterParameterProviderClass(PClass);
				}
			}
		}
		
	}
}

void USussGameSubsystem::RegisterNativeProviders()
{
	// Register any concrete action/query/input providers we supply in this lib (rather than as assets or things in settings)
	RegisterActionClass(USussActivateAbilityAction::StaticClass());
	
	RegisterInputProviderClass(USussTargetDistanceInputProvider::StaticClass());
	RegisterInputProviderClass(USussLocationDistanceInputProvider::StaticClass());
	RegisterInputProviderClass(USussTargetDistance2DInputProvider::StaticClass());
	RegisterInputProviderClass(USussLocationDistance2DInputProvider::StaticClass());
	RegisterInputProviderClass(USussTargetDistancePathInputProvider::StaticClass());
	RegisterInputProviderClass(USussLocationDistancePathInputProvider::StaticClass());
	RegisterInputProviderClass(USussSelfSightRangeInputProvider::StaticClass());
	RegisterInputProviderClass(USussSelfHearingRangeInputProvider::StaticClass());
	RegisterInputProviderClass(USussLineOfSightToTargetInputProvider::StaticClass());
	RegisterInputProviderClass(USussBlackboardAutoInputProvider::StaticClass());
	RegisterInputProviderClass(USussBlackboardBoolInputProvider::StaticClass());
	RegisterInputProviderClass(USussBlackboardFloatInputProvider::StaticClass());
	RegisterInputProviderClass(USussCanActivateAbilityInputProvider::StaticClass());
	RegisterInputProviderClass(USussTimeSinceActionPerformedInputProvider::StaticClass());
	
	RegisterQueryProviderClass(USussPerceptionKnownTargetsQueryProvider::StaticClass());
	RegisterQueryProviderClass(USussPerceptionKnownHostilesQueryProvider::StaticClass());
	RegisterQueryProviderClass(USussPerceptionKnownNonHostilesQueryProvider::StaticClass());
	RegisterQueryProviderClass(USussPerceptionKnownHostilesExtendedQueryProvider::StaticClass());

}


void USussGameSubsystem::RegisterActionClass(TSubclassOf<USussAction> ActionClass)
{
	if (auto CDO = GetDefault<USussAction>(ActionClass.Get()))
	{
		auto& Tag = CDO->GetActionTag();
		if (Tag.IsValid())
		{
			if (auto pExisting = ActionClasses.Find(Tag))
			{
				UE_LOG(LogSuss,
					   Error,
					   TEXT("Unable to register Action Class %s, tag %s already registered (%s)"),
					   *ActionClass->GetName(),
					   *Tag.GetTagName().ToString(),
					   *(*pExisting)->GetName())
			}
			else
			{
				ActionClasses.Add(Tag, ActionClass);
				UE_LOG(LogSuss, Log, TEXT("Registered Action Class %s for tag %s"), *ActionClass->GetName(), *Tag.GetTagName().ToString());
			}
		}
		else
		{
			UE_LOG(LogSuss, Warning, TEXT("Unable to register Action Class %s, tag is invalid. Did you forget to set it in your class defaults? If this is an abstract base class, mark it as abstract to remove this warning."), *ActionClass->GetName())
		}
	}
}

void USussGameSubsystem::UnregisterActionClass(TSubclassOf<USussAction> ActionClass)
{
	if (auto CDO = GetDefault<USussAction>(ActionClass.Get()))
	{
		auto& Tag = CDO->GetActionTag();
		if (Tag.IsValid())
		{
			if (auto pExisting = ActionClasses.Find(Tag))
			{
				if (*pExisting == ActionClass)
				{
					QueryProviders.Remove(Tag);
					return;
				}
			}
		}

		UE_LOG(LogSuss, Warning, TEXT("Possibly bad call to UnregisterActionClass, %s was not registered"), *ActionClass->GetName());
	}
}

TSubclassOf<USussAction> USussGameSubsystem::GetActionClass(FGameplayTag ActionTag)
{
	if (auto pClass = ActionClasses.Find(ActionTag))
	{
		return *pClass;
	}

	if (!MissingTagsAlreadyWarnedAbout.Contains(ActionTag.GetTagName()))
	{
		UE_LOG(LogSuss, Warning, TEXT("Requested non-existent Action Class for tag %s"), *ActionTag.GetTagName().ToString())
		MissingTagsAlreadyWarnedAbout.Add(ActionTag.GetTagName());
	}

	return nullptr;

}

void USussGameSubsystem::RegisterInputProvider(USussInputProvider* Provider)
{
	const auto Tag = Provider->GetInputTag();
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
		UE_LOG(LogSuss, Error, TEXT("Unable to register Input Provider %s, tag is invalid. Did you forget to set it in your class defaults? If this is an abstract base class, mark it as abstract to remove this warning."), *Provider->GetName())
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
	const auto Tag = Provider->GetQueryTag();
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
		UE_LOG(LogSuss, Error, TEXT("Unable to register Query Provider %s, tag is invalid. Did you forget to set it in your class defaults? If this is an abstract base class, mark it as abstract to remove this warning."), *Provider->GetName())
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
	const auto Tag = Provider->GetQueryTag();
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

void USussGameSubsystem::RegisterParameterProvider(USussParameterProvider* Provider)
{
	const auto Tag = Provider->GetParameterTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = ParameterProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
				   Error,
				   TEXT("Unable to register Parameter Provider %s, tag %s already registered (%s)"),
				   *Provider->GetName(),
				   *Tag.GetTagName().ToString(),
				   *(*pExisting)->GetName())
		}
		else
		{
			ParameterProviders.Add(Tag, Provider);
			UE_LOG(LogSuss, Log, TEXT("Registered Parameter Provider %s for tag %s"), *Provider->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Parameter Provider %s, tag is invalid. Did you forget to set it in your class defaults? If this is an abstract base class, mark it as abstract to remove this warning."), *Provider->GetName())
	}
}

void USussGameSubsystem::RegisterParameterProviderClass(TSubclassOf<USussParameterProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterParameterProvider, invalid class (no CDO)"))
	}
	RegisterParameterProvider(CDO);
	
}

void USussGameSubsystem::UnregisterParameterProvider(USussParameterProvider* Provider)
{
	const auto Tag = Provider->GetParameterTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = ParameterProviders.Find(Tag))
		{
			if (*pExisting == Provider)
			{
				ParameterProviders.Remove(Tag);
				return;
			}
		}
	}

	UE_LOG(LogSuss, Warning, TEXT("Possibly bad call to UnregisterParameterProvider, %s was not registered"), *Provider->GetName());

}

void USussGameSubsystem::UnregisterParameterProviderClass(TSubclassOf<USussParameterProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to UnregisterParameterProvider, invalid class %s (no CDO)"), *ProviderClass->GetName())
	}
	UnregisterParameterProvider(CDO);
}

USussParameterProvider* USussGameSubsystem::GetParameterProvider(const FGameplayTag& Tag)
{
	if (auto pProvider = ParameterProviders.Find(Tag))
	{
		return *pProvider;
	}

	if (!MissingTagsAlreadyWarnedAbout.Contains(Tag.GetTagName()))
	{
		UE_LOG(LogSuss, Warning, TEXT("Requested non-existent Parameter Provider for tag %s"), *Tag.GetTagName().ToString())
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

