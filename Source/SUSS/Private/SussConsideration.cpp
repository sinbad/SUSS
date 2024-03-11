
#include "SussConsideration.h"

#include "SussUtility.h"


float FSussConsideration::EvaluateCurve(float Input) const
{
	if (CurveType == ESussCurveType::Custom)
	{
		if (IsValid(CustomCurve))
		{
			return CustomCurve->GetFloatValue(Input);
		}
	}
	else
	{
		return USussUtility::EvalCurve(CurveType, Input, CurveParams);
	}

	return 0;	
}
