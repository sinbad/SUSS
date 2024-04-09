#pragma once
#include "NativeGameplayTags.h"

DECLARE_LOG_CATEGORY_CLASS(LogSuss, Log, Log)
DECLARE_STATS_GROUP(TEXT("SUSS"), STATGROUP_SUSS, STATCAT_Advanced);

// Create all the parent tags so they're always available
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussActionParentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussInputParentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussQueryParentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SussParamParentTag);

UENUM(BlueprintType)
enum class ESussCurveType : uint8
{
	/// Step function that goes from low to high in one go (at 0.5 base)
	Step,
	Linear,
	Exponential,
	Quadratic,
	/// S-shaped function
	Logistic,
	Custom
};

namespace SUSS
{
	extern const FName KeyParamName;
	extern const FName TagParamName;
	extern const FName AllowRemoteParamName;
	extern const FName CompletionDelayParamName;
}

