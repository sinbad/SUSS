#include "SussUtility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SussSettings.h"


bool USussUtility::IsActionEnabled(TSubclassOf<USussAction> ActionClass)
{
	if (auto Settings = GetDefault<USussSettings>())
	{
		return !Settings->DisabledActions.Contains(ActionClass);
	}

	return true;
}

bool USussUtility::ActorHasTag(AActor* Actor, const FGameplayTag& Tag)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasMatchingGameplayTag(Tag);
	}

	return false;
}

bool USussUtility::ActorHasAnyTags(AActor* Actor, const FGameplayTagContainer& Tags)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasAnyMatchingGameplayTags(Tags);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasAnyMatchingGameplayTags(Tags);
	}

	return false;
}

bool USussUtility::ActorHasAllTags(AActor* Actor, const FGameplayTagContainer& Tags)
{
	// Prefer Ability system if present
	if (UAbilitySystemComponent* const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return ASC->HasAllMatchingGameplayTags(Tags);
	}
	// GAS uses the C++-only cast approach to interfaces, not the BP-compatible Execute_Foo route
	else if (auto TI = Cast<IGameplayTagAssetInterface>(Actor))
	{
		return TI->HasAllMatchingGameplayTags(Tags);
	}

	return false;
}
#define PARAM_M Params.X
#define PARAM_K Params.Y
#define PARAM_B Params.Z
#define PARAM_C Params.W

float USussUtility::EvalStepCurve(float Input, const FVector4f& Params)
{
	// floor((x - c) * m * 2) + b
	return FMath::Floor((Input - PARAM_C) * PARAM_M * 2.f) + PARAM_B;
}

float USussUtility::EvalLinearCurve(float Input, const FVector4f& Params)
{
	// m * (x - c) + b
	return PARAM_M * (Input - PARAM_C) + PARAM_B;
}

float USussUtility::EvalQuadraticCurve(float Input, const FVector4f& Params)
{
	// m * (x - c)^k + b
	return PARAM_M * FMath::Pow(Input - PARAM_C, PARAM_K) + PARAM_B;
}

float USussUtility::EvalExponentialCurve(float Input, const FVector4f& Params)
{
	// m^(kx - c) + b
	return FMath::Pow(PARAM_M, PARAM_K * Input - PARAM_C) + PARAM_B;
}

float USussUtility::EvalLogisticCurve(float Input, const FVector4f& Params)
{
	// k * (1/(1+( (1000*e*m)^(-1 * x + c))) + b
	return PARAM_K * (1.0f / (1.0f+(FMath::Pow(1000.0f*UE_EULERS_NUMBER*PARAM_M, -1.0f * Input + PARAM_C)))) + PARAM_B;
}

float USussUtility::EvalCurve(ESussCurveType CurveType, float Input, const FVector4f& Params)
{
	switch(CurveType)
	{
	case ESussCurveType::Step:
		return EvalStepCurve(Input, Params);
	case ESussCurveType::Linear:
		return EvalLinearCurve(Input, Params);
	case ESussCurveType::Exponential:
		return EvalExponentialCurve(Input, Params);
	case ESussCurveType::Quadratic:
		return EvalQuadraticCurve(Input, Params);
	case ESussCurveType::Logistic:
		return EvalLogisticCurve(Input, Params);
	case ESussCurveType::Custom:
		checkf(false, TEXT("USussUtility::EvalCurve is not valid for custom curves"));
		break;
	}
	return 0;
}
