
#include "Queries/SussEnvQueryTest_TraceExtended.h"

void USussEnvQueryTest_TraceExtended::RunTest(FEnvQueryInstance& QueryInstance) const
{
	// Copied from superclass since we can't call it
	UObject* DataOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(DataOwner, QueryInstance.QueryID);
	TraceFromContext.BindData(DataOwner, QueryInstance.QueryID);
	ItemHeightOffset.BindData(DataOwner, QueryInstance.QueryID);
	ContextHeightOffset.BindData(DataOwner, QueryInstance.QueryID);
	
	bool bWantsHit = BoolValue.GetValue();
	bool bTraceToItem = TraceFromContext.GetValue();
	float ItemZ = ItemHeightOffset.GetValue();
	float ContextZ = ContextHeightOffset.GetValue();

	// SUSS begin
	ContextTraceOffset.BindData(DataOwner, QueryInstance.QueryID);
	ItemTraceOffset.BindData(DataOwner, QueryInstance.QueryID);
	const float ContextOffsetVal = ContextTraceOffset.GetValue();
	const float ItemOffsetVal = ItemTraceOffset.GetValue();
	// SUSS end

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(EnvQueryTrace), TraceData.bTraceComplex);

	TArray<AActor*> IgnoredActors;
	if (QueryInstance.PrepareContext(Context, IgnoredActors))
	{
		TraceParams.AddIgnoredActors(IgnoredActors);
	}
	
	ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);	
	FVector TraceExtent(TraceData.ExtentX, TraceData.ExtentY, TraceData.ExtentZ);
	FRunTraceSignature TraceFunc;
	switch (TraceData.TraceShape)
	{
	case EEnvTraceShape::Line:
		TraceFunc.BindUObject(const_cast<USussEnvQueryTest_TraceExtended*>(this), bTraceToItem ? &USussEnvQueryTest_TraceExtended::RunLineTraceTo : &USussEnvQueryTest_TraceExtended::RunLineTraceFrom);
		break;

	case EEnvTraceShape::Box:
		TraceFunc.BindUObject(const_cast<USussEnvQueryTest_TraceExtended*>(this), bTraceToItem ? &USussEnvQueryTest_TraceExtended::RunBoxTraceTo : &USussEnvQueryTest_TraceExtended::RunBoxTraceFrom);
		break;

	case EEnvTraceShape::Sphere:
		TraceFunc.BindUObject(const_cast<USussEnvQueryTest_TraceExtended*>(this), bTraceToItem ? &USussEnvQueryTest_TraceExtended::RunSphereTraceTo : &USussEnvQueryTest_TraceExtended::RunSphereTraceFrom);
		break;

	case EEnvTraceShape::Capsule:
		TraceFunc.BindUObject(const_cast<USussEnvQueryTest_TraceExtended*>(this), bTraceToItem ? &USussEnvQueryTest_TraceExtended::RunCapsuleTraceTo : &USussEnvQueryTest_TraceExtended::RunCapsuleTraceFrom);
		break;

	default:
		return;
	}

	for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
	{
		ContextLocations[ContextIndex].Z += ContextZ;
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// SUSS begin
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex()) + FVector(0, 0, ItemZ);
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			FVector ContextLocation = ContextLocations[ContextIndex];
			// Offset the ends if needed
			if (!FMath::IsNearlyZero(ContextOffsetVal) || !FMath::IsNearlyZero(ItemOffsetVal))
			{
				FVector ContextToItemVec =  ItemLocation - ContextLocation;
				ContextToItemVec.Normalize();
				ContextLocation += ContextOffsetVal * ContextToItemVec;
				ItemLocation -= ItemOffsetVal * ContextToItemVec;
			}

			// SUSS end
			
			const bool bHit = TraceFunc.Execute(ItemLocation, ContextLocation, ItemActor, QueryInstance.World, TraceCollisionChannel, TraceParams, TraceExtent);
			It.SetScore(TestPurpose, FilterType, bHit, bWantsHit);
		}
	}
}
