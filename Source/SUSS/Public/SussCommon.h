#pragma once

DECLARE_LOG_CATEGORY_CLASS(LogSuss, Log, Log)
DECLARE_STATS_GROUP(TEXT("SUSS"), STATGROUP_SUSS, STATCAT_Advanced);

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


