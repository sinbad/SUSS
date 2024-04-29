
#include "Queries/SussEQSQueryProvider.h"

#include "SussBrainComponent.h"
#include "SussUtility.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Misc/TransactionObjectEvent.h"
#include "Queries/SussEQSWorldSubsystem.h"

TSharedPtr<FEnvQueryResult> USussEQSQueryProvider::RunEQSQuery(USussBrainComponent* Brain,
                                                               AActor* Self,
                                                               const TMap<FName, FSussParameter>& Params,
                                                               const FSussContext& Context)
{
	TArray<FEnvNamedValue> QueryParams = QueryConfig;
	USussUtility::AddEQSParams(Params, QueryParams);

	// unfortunately we have no place to store any extra EQS context values like current target
	// we use this subsystem hack instead
	bool bClearTargetContext = false;
	if (bIsCorrelatedWithContext)
	{
		auto EQSSub = Self->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		if (Context.Target.IsValid())
		{
			EQSSub->SetTargetInfo(Self, Context.Target.Get());
			bClearTargetContext = true;
		}
	}
	TSharedPtr<FEnvQueryResult> Ret = USussUtility::RunEQSQuery(Self, EQSQuery, QueryParams, QueryMode);

	// Clear temp context info
	if (bClearTargetContext)
	{
		auto EQSSub = Self->GetWorld()->GetSubsystem<USussEQSWorldSubsystem>();
		EQSSub->ClearTargetInfo(Self);
	}
	return Ret;
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
	const auto Result = RunEQSQuery(Brain, Self, Params, Context);
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
	const auto Result = RunEQSQuery(Brain, Self, Params, Context);
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
