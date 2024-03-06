
#include "SussGameSubsystem.h"

#include "SussCommon.h"
#include "SussDummyProviders.h"
#include "SussSettings.h"

void USussGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DefaultInputProvider = NewObject<USussDummyInputProvider>();

	// Auto set up based on settings
	if (auto Settings = GetDefault<USussSettings>())
	{
		for (auto& InputProv : Settings->InputProviders)
		{
			RegisterInputProvider(InputProv);
		}
		for (auto& QProv : Settings->QueryProviders)
		{
			RegisterQueryProvider(QProv);
		}
	}
}

void USussGameSubsystem::RegisterInputProvider(TSubclassOf<USussInputProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterInputProvider, invalid class (no CDO)"))
	}
	auto& Tag = CDO->GetInputTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = InputProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
			       Error,
			       TEXT("Unable to register Input Provider %s, tag %s already registered (%s)"),
			       *CDO->GetName(),
			       *Tag.GetTagName().ToString(),
			       *(*pExisting)->GetName())
		}
		else
		{
			InputProviders.Add(Tag, CDO);
			UE_LOG(LogSuss, Log, TEXT("Registered Input Provider %s for tag %s"), *CDO->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Input Provider %s, gameplay tag is invalid, did you remember to set it in your input provider?"), *CDO->GetName())
	}
	
}

USussInputProvider* USussGameSubsystem::GetInputProvider(const FGameplayTag& Tag)
{
	if (auto pProvider = InputProviders.Find(Tag))
	{
	}

	if (!MissingTagsAlreadyWarnedAbout.Contains(Tag.GetTagName()))
	{
		UE_LOG(LogSuss, Warning, TEXT("Requested non-existent Input Provider for tag %s"), *Tag.GetTagName().ToString())
		MissingTagsAlreadyWarnedAbout.Add(Tag.GetTagName());
	}
	return DefaultInputProvider;
}


void USussGameSubsystem::RegisterQueryProvider(TSubclassOf<USussQueryProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterQueryProvider, invalid class (no CDO)"))
	}
	auto& Tag = CDO->GetQueryTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = QueryProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
				   Error,
				   TEXT("Unable to register Query Provider %s, tag %s already registered (%s)"),
				   *CDO->GetName(),
				   *Tag.GetTagName().ToString(),
				   *(*pExisting)->GetName())
		}
		else
		{
			QueryProviders.Add(Tag, CDO);
			UE_LOG(LogSuss, Log, TEXT("Registered Query Provider %s for tag %s"), *CDO->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Query Provider %s, gameplay tag is invalid, did you remember to set it in your input provider?"), *CDO->GetName())
	}
	
}

USussQueryProvider* USussGameSubsystem::GetQueryProvider(const FGameplayTag& Tag)
{
	if (auto pProvider = QueryProviders.Find(Tag))
	{
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
	return GetGameInstance()->GetWorld();
}

void USussGameSubsystem::Tick(float DeltaTime)
{
	// Tick the queries so they know when to invalidate results
	for (auto& Pair : QueryProviders)
	{
		Pair.Value->Tick(DeltaTime);
	}
}
