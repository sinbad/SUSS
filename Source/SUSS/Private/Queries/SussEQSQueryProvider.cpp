
#include "Queries/SussEQSQueryProvider.h"

#include "SussUtility.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Misc/TransactionObjectEvent.h"

TSharedPtr<FEnvQueryResult> USussEQSQueryProvider::RunEQSQuery(USussBrainComponent* Brain,
                                        AActor* Self,
                                        const TMap<FName, FSussParameter>& Params)
{
	TArray<FEnvNamedValue> QueryParams = QueryConfig;
	USussUtility::AddEQSParams(Params, QueryParams);
	return USussUtility::RunEQSQuery(Self, EQSQuery, QueryParams, QueryMode);
}

bool USussEQSQueryProvider::ShouldIncludeResult(const FEnvQueryItem& Item) const
{
	return MinScore <= 0 || Item.Score >= MinScore;
}

void USussEQSTargetQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                               AActor* Self,
                                               const TMap<FName, FSussParameter>& Params,
                                               const FSussContext& Context,
                                               TArray<TWeakObjectPtr<AActor>>& OutResults)
{
	const auto Result = RunEQSQuery(Brain, Self, Params);
	if (Result->ItemType->IsChildOf(UEnvQueryItemType_ActorBase::StaticClass()))
	{
		const UEnvQueryItemType_ActorBase* DefTypeOb =  Result->ItemType->GetDefaultObject<UEnvQueryItemType_ActorBase>();
		for (const auto& Item : Result->Items)
		{
			if (ShouldIncludeResult(Item))
			{
				OutResults.Add(DefTypeOb->GetActor(Result->RawData.GetData() + Item.DataOffset));
			}
		}
	}
}

void USussEQSLocationQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                                 AActor* Self,
                                                 const TMap<FName, FSussParameter>& Params,
                                                 const FSussContext& Context,
                                                 TArray<FVector>& OutResults)
{
	const auto Result = RunEQSQuery(Brain, Self, Params);
	if (Result->ItemType->IsChildOf(UEnvQueryItemType_VectorBase::StaticClass()))
	{
		const UEnvQueryItemType_VectorBase* DefTypeOb =  Result->ItemType->GetDefaultObject<UEnvQueryItemType_VectorBase>();
		for (const auto& Item : Result->Items)
		{
			if (ShouldIncludeResult(Item))
			{
				OutResults.Add(DefTypeOb->GetItemLocation(Result->RawData.GetData() + Item.DataOffset));
			}
		}
	}
}


#if WITH_EDITOR
void USussEQSQueryProvider::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		OnPropertyChanged(PropertyChangedEvent.MemberProperty->GetFName());
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void USussEQSQueryProvider::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);

	if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
	{
		if (TransactionEvent.GetChangedProperties().Num() > 0)
		{
			// targeted update
			for (const FName PropertyName : TransactionEvent.GetChangedProperties())
			{
				OnPropertyChanged(PropertyName);
			}
		}
	}
}

void USussEQSQueryProvider::OnPropertyChanged(const FName PropName)
{
	// This is to update our parameter list based on the query selected
	static const FName NAME_Query = GET_MEMBER_NAME_CHECKED(USussEQSQueryProvider, EQSQuery);
	static const FName NAME_QueryConfig = GET_MEMBER_NAME_CHECKED(USussEQSQueryProvider, QueryConfig);

	if (PropName == NAME_Query || PropName == NAME_QueryConfig)
	{
		if (EQSQuery)
		{
			TArray<FAIDynamicParam> DParams;
			EQSQuery->CollectQueryParams(*this, DParams);

			QueryConfig.Empty();
			for (auto& DParam : DParams)
			{
				FEnvNamedValue NamedValue;
				NamedValue.ParamName = DParam.ParamName;
				NamedValue.ParamType = DParam.ParamType;
				NamedValue.Value = DParam.Value;
				QueryConfig.Add( NamedValue);
			}
		}
	}
}
#endif // WITH_EDITOR
