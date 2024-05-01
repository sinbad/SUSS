#include "SussUtility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NavigationSystem.h"
#include "SussGameSubsystem.h"
#include "SussSettings.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Queries/SussEQSWorldSubsystem.h"


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
		case ESussParamType::Bool:
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

UEnvQueryInstanceBlueprintWrapper* USussUtility::RunEQSQueryBP(AActor* Querier,
	UEnvQuery* EQSQuery,
	const TArray<FEnvNamedValue>& QueryParams,
	EEnvQueryRunMode::Type QueryMode)
{
	UWorld* World = GEngine->GetWorldFromContextObject(Querier, EGetWorldErrorMode::LogAndReturnNull);

	if (!EQSQuery || !World)
		return nullptr;

	if (UEnvQueryManager* EQS = UEnvQueryManager::GetCurrent(World))
	{
		auto Wrapper = NewObject<UEnvQueryInstanceBlueprintWrapper>(UEnvQueryManager::GetCurrent(Querier), UEnvQueryInstanceBlueprintWrapper::StaticClass());
		check(Wrapper);
		Wrapper->SetInstigator(Querier);

		FEnvQueryRequest QueryRequest(EQSQuery, Querier);
		QueryRequest.SetNamedParams(QueryParams);
		Wrapper->RunQuery(QueryMode, QueryRequest);
		return Wrapper;
	}

	return nullptr;
	
}

TSharedPtr<FEnvQueryResult> USussUtility::RunEQSQueryWithTargetContext(AActor* Querier,
                                                                       AActor* Target,
                                                                       UEnvQuery* EQSQuery,
                                                                       const TMap<FName, FSussParameter>& Params,
                                                                       TEnumAsByte<EEnvQueryRunMode::Type> QueryMode)
{
	// unfortunately we have no place to store any extra EQS context values like current target
	// we use this subsystem hack instead
	bool bClearTargetContext = false;
	if (IsValid(Target))
	{
		auto EQSSub = Querier->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		EQSSub->SetTargetInfo(Querier, Target);
		bClearTargetContext = true;
	}
	TArray<FEnvNamedValue> QueryParams;
	USussUtility::AddEQSParams(Params, QueryParams);
	TSharedPtr<FEnvQueryResult> Ret = RunEQSQuery(Querier, EQSQuery, QueryParams, QueryMode);

	// Clear temp context info
	if (bClearTargetContext)
	{
		auto EQSSub = Querier->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		EQSSub->ClearTargetInfo(Querier);
	}
	return Ret;
}

UEnvQueryInstanceBlueprintWrapper* USussUtility::RunEQSQueryWithTargetContextBP(AActor* Querier,
	AActor* Target,
	UEnvQuery* EQSQuery,
	const TMap<FName, FSussParameter>& Params,
	TEnumAsByte<EEnvQueryRunMode::Type> QueryMode)
{
	UEnvQueryInstanceBlueprintWrapper* Wrapper = nullptr;

	if (IsValid(Querier) && EQSQuery)
	{
		// unfortunately we have no place to store any extra EQS context values like current target
		// we use this subsystem hack instead
		auto EQSSub = Querier->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		if (Target)
		{
			EQSSub->SetTargetInfo(Querier, Target);
		}
		TArray<FEnvNamedValue> QueryParams;
		USussUtility::AddEQSParams(Params, QueryParams);
		auto Ret = RunEQSQuery(Querier, EQSQuery, QueryParams, QueryMode);

		// Clear temp context info
		EQSSub->ClearTargetInfo(Querier);

		Wrapper = NewObject<UEnvQueryInstanceBlueprintWrapper>(UEnvQueryManager::GetCurrent(Querier), UEnvQueryInstanceBlueprintWrapper::StaticClass());
		check(Wrapper);
		Wrapper->SetInstigator(Querier);

		FEnvQueryRequest QueryRequest(EQSQuery, Querier);
		QueryRequest.SetNamedParams(QueryParams);
		Wrapper->RunQuery(QueryMode, QueryRequest);
		
	}
	
	return Wrapper;
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
	case ESussParamType::Bool:
		// Allow conversion
		Value = Parameter.BoolValue ? 1.0f : 0.0f;
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
	case ESussParamType::Bool:
	case ESussParamType::Float:
	case ESussParamType::Int:
	case ESussParamType::Name:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
		return false;
	};

	return false;
}

bool USussUtility::GetSussParameterValueAsBool(const FSussParameter& Parameter, bool& Value)
{
	switch (Parameter.Type)
	{
	case ESussParamType::Bool:
		Value = Parameter.BoolValue;
		return true;
	case ESussParamType::Float:
		// Allow conversion
		Value = !FMath::IsNearlyZero(Parameter.FloatValue);
		return true;
	case ESussParamType::Int:
		// Allow conversion
		Value = Parameter.IntValue != 0;
		return true;
	case ESussParamType::Vector:
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
	case ESussParamType::Bool:
		// Allow conversion
		Value = Parameter.BoolValue ? 1 : 0;
		return true;
	case ESussParamType::Name:
	case ESussParamType::Tag:
	case ESussParamType::AutoParameter:
	case ESussParamType::Vector:
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
	case ESussParamType::Vector:
	case ESussParamType::Bool:
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
	case ESussParamType::Vector:
	case ESussParamType::Bool:
		return false;
	};

	return false;
}

bool USussUtility::GetSussContextValueAsActor(const FSussContext& Context, FName Name, AActor*& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{
		case ESussContextValueType::Actor:
			Value = pValue->Value.Get<TWeakObjectPtr<AActor>>().Get();
			return true;
		case ESussContextValueType::Int:
		case ESussContextValueType::NONE:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Float:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Tag:
		case ESussContextValueType::Name:
		case ESussContextValueType::Struct:
			return false;
		}
	}

	return false;	
}

bool USussUtility::GetSussContextValueAsFloat(const FSussContext& Context, FName Name, float& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{
		case ESussContextValueType::Float:
			Value = pValue->Value.Get<float>();
			return true;
		case ESussContextValueType::Int:
			// Allow conversion
			Value = pValue->Value.Get<int>();
			return true;
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Tag:
		case ESussContextValueType::Struct:
		case ESussContextValueType::Name:
			return false;
		}
	}

	return false;
}

bool USussUtility::GetSussContextValueAsVector(const FSussContext& Context, FName Name, FVector& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{
		case ESussContextValueType::Vector:
			Value = pValue->Value.Get<FVector>();
			return true;
		case ESussContextValueType::Int:
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Float:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Tag:
		case ESussContextValueType::Name:
		case ESussContextValueType::Struct:
			return false;
		}
	}

	return false;
}

bool USussUtility::GetSussContextValueAsRotator(const FSussContext& Context, FName Name, FRotator& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{
		case ESussContextValueType::Rotator:
			Value = pValue->Value.Get<FRotator>();
			return true;
		case ESussContextValueType::Int:
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Float:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Tag:
		case ESussContextValueType::Name:
		case ESussContextValueType::Struct:
			return false;
		}
	}

	return false;	
}


bool USussUtility::GetSussContextValueAsInt(const FSussContext& Context, FName Name, int& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{
		case ESussContextValueType::Int:
			Value = pValue->Value.Get<int>();
			return true;
		case ESussContextValueType::Float:
			// Allow conversion
			Value = pValue->Value.Get<float>();
			return true;
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Tag:
		case ESussContextValueType::Name:
		case ESussContextValueType::Struct:
			return false;
		}
	}

	return false;
}

bool USussUtility::GetSussContextValueAsName(const FSussContext& Context, FName Name, FName& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{

		case ESussContextValueType::Tag:
			// Allow conversion
			Value = pValue->Value.Get<FGameplayTag>().GetTagName();
			return true;
		case ESussContextValueType::Name:
			Value = pValue->Value.Get<FName>();
			return true;
		case ESussContextValueType::Int:
		case ESussContextValueType::Float:
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Struct:
			return false;
		}
	}

	return false;
}

bool USussUtility::GetSussContextValueAsTag(const FSussContext& Context, FName Name, FGameplayTag& Value)
{
	if (const auto pValue = Context.NamedValues.Find(Name))
	{
		switch(pValue->Type)
		{

		case ESussContextValueType::Tag:
			Value = pValue->Value.Get<FGameplayTag>();
			return true;
		case ESussContextValueType::Name:
		case ESussContextValueType::Int:
		case ESussContextValueType::Float:
		case ESussContextValueType::NONE:
		case ESussContextValueType::Actor:
		case ESussContextValueType::Vector:
		case ESussContextValueType::Rotator:
		case ESussContextValueType::Struct:
			return false;
		}
	}

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

const TArray<FVector>& USussUtility::RunLocationQueryWithTargetContext(AActor* Querier,
	FGameplayTag Tag,
	AActor* Target,
	const TMap<FName, FSussParameter>& Params,
	float UseCachedResultsFor)
{
	if (IsValid(Querier))
	{
		// unfortunately we have no place to store any extra EQS context values like current target
		// we use this subsystem hack instead
		auto EQSSub = Querier->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		if (Target)
		{
			EQSSub->SetTargetInfo(Querier, Target);
		}

		const TArray<FVector>& Ret =RunLocationQuery(Querier, Tag, Params, UseCachedResultsFor);

		// Clear temp context info
		EQSSub->ClearTargetInfo(Querier);

		return Ret;
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

float USussUtility::GetPathDistanceTo(AAIController* Agent, const FVector& Location)
{
	if (Agent && Agent->GetPawn())
	{
		return GetPathDistanceFromTo(Agent, Agent->GetPawn()->GetActorLocation(), Location);
	}
	return BIG_NUMBER;
}

float USussUtility::GetPathDistanceFromTo(AAIController* Agent, const FVector& FromLocation, const FVector& ToLocation)
{
	if (!Agent)
	{
		return BIG_NUMBER;
	}
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Agent->GetWorld()))
	{
		const ANavigationData* NavData;
		INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Agent);
		if (NavAgent)
		{
			NavData = NavSys->GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
		}
		else
		{
			NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
		}
	
		if (NavData)
		{
			FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, Agent->GetDefaultNavigationFilterClass());
			FPathFindingQuery Query(Agent, *NavData, FromLocation, ToLocation, NavFilter);
			Query.SetAllowPartialPaths(false);
			auto Result = NavSys->FindPathSync(Query);

			if (Result.IsSuccessful())
			{
				return Result.Path->GetLength();
			}
		}
	}

	return BIG_NUMBER;
}
