#pragma once

#include "CoreMinimal.h"

/// Real-time performance counter using cycles
struct FSussScopedPerfTimer
{
protected:
	uint64 StartCycles;
	uint64 WrapAdjust = 0;
public:
	
	FSussScopedPerfTimer()
	{
		StartCycles = FPlatformTime::Cycles64();
	}

	double Seconds()
	{
		const uint64 CurrCycles = FPlatformTime::Cycles64();
		// Detect wrap
		if (CurrCycles < StartCycles)
		{
			WrapAdjust = std::numeric_limits<uint64>::max() - StartCycles;
			StartCycles = 0;
		}

		return FPlatformTime::GetSecondsPerCycle64() * (WrapAdjust + (CurrCycles - StartCycles));
	}

	double Milliseconds()
	{
		return Seconds() * 1000.0;
	}
};
