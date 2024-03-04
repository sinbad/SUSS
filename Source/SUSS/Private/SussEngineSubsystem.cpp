
#include "SussEngineSubsystem.h"

#include "SussCommon.h"
#include "SussDummyProviders.h"

void USussEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DefaultInputProvider = NewObject<USussDummyInputProvider>();
}

void USussEngineSubsystem::RegisterInputProvider(TSubclassOf<USussInputProvider> ProviderClass)
{
	const auto CDO = ProviderClass.GetDefaultObject();

	if (!CDO)
	{
		UE_LOG(LogSuss, Error, TEXT("Bad call to RegisterInputProvider, invalid class (no CDO)"))
	}
	auto& Tag = CDO->GetGameplayTag();
	if (Tag.IsValid())
	{
		if (auto pExisting = InputProviders.Find(Tag))
		{
			UE_LOG(LogSuss,
			       Error,
			       TEXT("Unable to register Input Provider %s, tag %s already registered (%s)"),
			       *CDO->GetName(),
			       *Tag.GetTagName().ToString(),
			       *pExisting->GetDefaultObject()->GetName())
		}
		else
		{
			UE_LOG(LogSuss, Log, TEXT("Registered Input Provider %s for tag %s"), *CDO->GetName(), *Tag.GetTagName().ToString());
		}
	}
	else
	{
		UE_LOG(LogSuss, Error, TEXT("Unable to register Input Provider %s, gameplay tag is invalid, did you remember to set it in your input provider?"), *CDO->GetName())
	}
	
}

USussInputProvider* USussEngineSubsystem::GetInputProvider(const FGameplayTag& Tag)
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
