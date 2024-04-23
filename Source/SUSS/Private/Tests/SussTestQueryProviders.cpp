#include "SussTestQueryProviders.h"

const FName USussTestSingleLocationQueryProvider::TagName("Suss.Query.Location.Test.Single");
const FName USussTestMultipleLocationQueryProvider::TagName("Suss.Query.Location.Test.Multiple");
const FName USussTestNamedLocationValueQueryProvider::TagName("Suss.Query.Test.Named.Location");
const FName USussTestNamedFloatValueQueryProvider::TagName("Suss.Query.Test.Named.Float");
const FName USussTestNamedStructSharedValueQueryProvider::TagName("Suss.Query.Test.Named.Struct.Shared");
const FName USussTestNamedStructRawPointerQueryProvider::TagName("Suss.Query.Test.Named.Struct.NonShared");
const FName USussTestCorrelatedNamedFloatValueQueryProvider::TagName("Suss.Query.Test.Named.Float.Correlated");

FSussTestQueryTagHolder FSussTestQueryTagHolder::Instance;

FSussTestQueryTagHolder::FSussTestQueryTagHolder()
{
}

FSussTestQueryTagHolder::~FSussTestQueryTagHolder()
{
	UnregisterTags();
}

FGameplayTag FSussTestQueryTagHolder::GetTag(FName TagName)
{
	if (const auto pTagPtr = Tags.Find(TagName))
	{
		return (*pTagPtr)->GetTag();
	}

	const auto& Ptr = Tags.Add(TagName, MakeUnique<FNativeGameplayTag>(UE_PLUGIN_NAME,
	                                                                   UE_MODULE_NAME,
	                                                                   TagName,
	                                                                   "",
	                                                                   ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD));
	return Ptr->GetTag();
}

void FSussTestQueryTagHolder::UnregisterTags()
{
	// Removing the TUniquePtr will delete the native tag & unregister
	Tags.Empty();
}
