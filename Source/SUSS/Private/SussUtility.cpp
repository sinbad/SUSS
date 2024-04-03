#include "SussUtility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SussGameSubsystem.h"
#include "SussSettings.h"
#include "EnvironmentQuery/EnvQueryManager.h"

UE_DISABLE_OPTIMIZATION

bool USussUtility::IsActionEnabled(FGameplayTag ActionTag)
{
	if (auto Settings = GetDefault<USussSettings>())
	{
		return !Settings->DisabledActionTags.HasTag(ActionTag);
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

void USussUtility::AddEQSParams(const TMap<FName, FSussParameter>& Params, TArray<FEnvNamedValue>& OutQueryParams)
{
	for (const auto& Param : Params)
	{
		const FSussParameter& InParam = Param.Value;
		FEnvNamedValue QParam;
		QParam.ParamName = Param.Key;
		switch (Param.Value.Type)
		{
		case ESussParamType::Float:
			QParam.ParamType = EAIParamType::Float;
			QParam.Value = InParam.FloatValue;
			break;
		case ESussParamType::Int:
			QParam.ParamType = EAIParamType::Int;
			QParam.Value = InParam.IntValue;
			break;
		case ESussParamType::Vector:
		case ESussParamType::Tag:
		case ESussParamType::Name:
		case ESussParamType::AutoParameter:
			// Not supported
			continue;
		}
		OutQueryParams.Add(QParam);
	}
}

TSharedPtr<FEnvQueryResult> USussUtility::RunEQSQuery(UObject* WorldContextObject,
                                                      UEnvQuery* EQSQuery,
                                                      const TArray<FEnvNamedValue>& QueryParams,
                                                      EEnvQueryRunMode::Type QueryMode)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!EQSQuery || !World)
		return nullptr;

	if (UEnvQueryManager* EQS = UEnvQueryManager::GetCurrent(World))
	{
		// We *could* allow EQS to be run in steps over many frames here
		// See ExecuteOneStep inside this function, or EQSTestingPawn
		// Or we could use RunQuery with callback.
		// For now, run synchronous for simplicity, and limit time between AIs
		FEnvQueryRequest QueryRequest(EQSQuery, WorldContextObject);
		QueryRequest.SetNamedParams(QueryParams);
		
		return EQS->RunInstantQuery(QueryRequest, QueryMode);
	}

	return nullptr;
}


bool USussUtility::GetSussParameterValueAsFloat(const FSussParameter& Parameter, float& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Float:
		Value = Parameter.FloatValue;
		return true;
	case ESussParamType::Int:
		// Allow conversion
		Value = Parameter.IntValue;
		return true;
	case ESussParamType::Name:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
	case ESussParamType::Vector:
		return false;
	};

	return false;
	
}

bool USussUtility::GetSussParameterValueAsVector(const FSussParameter& Parameter, FVector& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Vector:
		Value = Parameter.VectorValue;
		return true;
	case ESussParamType::Float:
	case ESussParamType::Int:
	case ESussParamType::Name:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
		return false;
	};

	return false;
}

bool USussUtility::GetSussParameterValueAsInt(const FSussParameter& Parameter, int& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Float:
		// Allow conversion
		Value = Parameter.FloatValue;
		return true;
	case ESussParamType::Int:
		Value = Parameter.IntValue;
		return true;
	case ESussParamType::Name:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
		return false;
	};

	return false;
}

bool USussUtility::GetSussParameterValueAsName(const FSussParameter& Parameter, FName& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Name:
		Value = Parameter.NameValue;
		return true;
	case ESussParamType::Float:
	case ESussParamType::Int:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
		return false;
	};

	return false;
}

bool USussUtility::GetSussParameterValueAsTag(const FSussParameter& Parameter, FGameplayTag& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Name:
		Value = Parameter.Tag;
		return true;
	case ESussParamType::AutoParameter:
		// Allow conversion
		Value = Parameter.InputOrParameterTag;
		return true;
	case ESussParamType::Float:
	case ESussParamType::Int:
	case ESussParamType::Tag:
		return false;
	};

	return false;
}

const TArray<FVector>& USussUtility::RunLocationQuery(AActor* Querier, FGameplayTag Tag, const TMap<FName, FSussParameter>& Params, float UseCachedResultsFor)
{
	if (IsValid(Querier))
	{
		if (auto SUSS = GetSUSS(Querier->GetWorld()))
		{
			if (const auto Provider = SUSS->GetQueryProvider(Tag))
			{
				if (Provider && Provider->GetProvidedContextElement() == ESussQueryContextElement::Location)
				{
					return Provider->GetResults<FVector>(nullptr, Querier, UseCachedResultsFor, Params);
				}
			}
		}
	}

	static TArray<FVector> DummyResults;
	return DummyResults;
}

TArray<AActor*> USussUtility::RunTargetQuery(AActor* Querier,
                                             FGameplayTag Tag,
                                             const TMap<FName, FSussParameter>& Params,
                                             float UseCachedResultsFor)
{
	// Sadly we can't expose weak pointers to BPs, will have to copy
	TArray<AActor*> Results;
	
	if (IsValid(Querier))
	{
		if (auto SUSS = GetSUSS(Querier->GetWorld()))
		{
			if (const auto Provider = SUSS->GetQueryProvider(Tag))
			{
				if (Provider && Provider->GetProvidedContextElement() == ESussQueryContextElement::Target)
				{
					const auto& WeakResults = Provider->GetResults<TWeakObjectPtr<AActor>>(
						nullptr,
						Querier,
						UseCachedResultsFor,
						Params);
					for (auto WeakActor : WeakResults)
					{
						if (WeakActor.IsValid())
						{
							Results.Add(WeakActor.Get());
						}
					}
				}
			}
		}
	}

	return Results;
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

UE_ENABLE_OPTIMIZATION