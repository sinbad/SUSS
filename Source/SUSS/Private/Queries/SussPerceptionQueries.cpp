#include "Queries/SussPerceptionQueries.h"

#include "SussBrainComponent.h"
#include "SussUtility.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownTargets, "Suss.Query.Perception.Targets.AllKnown", "Query all targets known to this agent's perception system. Optional param 'Sense' to filter ('Sight', 'Hearing' etc)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownHostiles, "Suss.Query.Perception.Targets.HostilesKnown", "Query all hostiles known to this agent's perception system. Optional param 'Sense' to filter ('Sight', 'Hearing' etc)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownNonHostiles, "Suss.Query.Perception.Targets.NonHostilesKnown", "Query all non-hostiles known to this agent's perception system. Optional param 'Sense' to filter ('Sight', 'Hearing' etc)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownHostilesExtended, "Suss.Query.Perception.Targets.HostilesKnown.Extended", "Query all hostiles known to this agent's perception system, return named value struct 'PerceptionInfo' of type FSussActorPerceptionInfo. Optional param 'Sense' to filter ('Sight', 'Hearing' etc)")


USussPerceptionKnownTargetsQueryProviderBase::USussPerceptionKnownTargetsQueryProviderBase()
{
	// Nothing to do here, not a concrete query
}

TSubclassOf<UAISense> USussPerceptionKnownTargetsQueryProviderBase::GetSenseClass(
	const TMap<FName, FSussParameter>& Params)
{
	return USussUtility::GetSenseClassFromParams(Params);
}

void USussPerceptionKnownTargetsQueryProviderBase::GetIgnoreTags(
	const TMap<FName, FSussParameter>& Params,
	FGameplayTagContainer& OutTags)
{
	USussUtility::GetIgnoreTagsFromParams(Params, OutTags);
}

USussPerceptionKnownTargetsQueryProvider::USussPerceptionKnownTargetsQueryProvider()
{
	QueryTag = TAG_SussQueryPerceptionKnownTargets;
}

void USussPerceptionKnownTargetsQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                                            AActor* Self,
                                                            const TMap<FName, FSussParameter>& Params,
                                                            const FSussContext& Context,
                                                            TArray<TWeakObjectPtr<AActor>>& OutResults)
{
	if (const auto Perception = Brain->GetPerceptionComponent())
	{
		TArray<AActor*> PerceptionResults;
		TSubclassOf<UAISense> SenseClass = GetSenseClass(Params);
		FGameplayTagContainer IgnoreTags;
		GetIgnoreTags(Params, IgnoreTags);
		if (SenseClass)
		{
			// Note that the "known" excludes forgotten actors but includes actors which are not *currently* perceived but
			// still remembered
			Perception->GetKnownPerceivedActors(SenseClass, PerceptionResults);
		}
		else
		{
			Perception->GetFilteredActors([](const FActorPerceptionInfo& Info)
			{
				return Info.HasAnyKnownStimulus();
			}, PerceptionResults);
		}
		for (const auto Actor : PerceptionResults)
		{
			if (!USussUtility::ActorHasAnyTags(Actor, IgnoreTags))
			{
				OutResults.Add(Actor);
			}
		}
	}
}

USussPerceptionKnownHostilesQueryProvider::USussPerceptionKnownHostilesQueryProvider()
{
	QueryTag = TAG_SussQueryPerceptionKnownHostiles;
}

void USussPerceptionKnownHostilesQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                                       AActor* Self,
                                                       const TMap<FName, FSussParameter>& Params,
                                                       const FSussContext& Context,
                                                       TArray<TWeakObjectPtr<AActor>>& OutResults)
{
	if (const auto Perception = Brain->GetPerceptionComponent())
	{
		TArray<AActor*> PerceptionResults;
		TSubclassOf<UAISense> SenseClass = GetSenseClass(Params);
		FGameplayTagContainer IgnoreTags;
		GetIgnoreTags(Params, IgnoreTags);

		if (SenseClass)
		{
			Perception->GetHostileActorsBySense(SenseClass, PerceptionResults);
		}
		else
		{
			Perception->GetHostileActors(PerceptionResults);
		}

		for (const auto Actor : PerceptionResults)
		{
			if (!USussUtility::ActorHasAnyTags(Actor, IgnoreTags))
			{
				OutResults.Add(Actor);
			}
		}
	}
}

USussPerceptionKnownNonHostilesQueryProvider::USussPerceptionKnownNonHostilesQueryProvider()
{
	QueryTag = TAG_SussQueryPerceptionKnownNonHostiles;
}

void USussPerceptionKnownNonHostilesQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
													   AActor* Self,
													   const TMap<FName, FSussParameter>& Params,
													   const FSussContext& Context,
													   TArray<TWeakObjectPtr<AActor>>& OutResults)
{
	if (const auto Perception = Brain->GetPerceptionComponent())
	{
		TArray<AActor*> PerceptionResults;
		TSubclassOf<UAISense> SenseClass = GetSenseClass(Params);
		FGameplayTagContainer IgnoreTags;
		GetIgnoreTags(Params, IgnoreTags);
		if (SenseClass)
		{
			const FAISenseID SenseID = UAISense::GetSenseID(SenseClass);
			Perception->GetFilteredActors([SenseID](const FActorPerceptionInfo& Info)
			{
				return !Info.bIsHostile && Info.HasKnownStimulusOfSense(SenseID);
			}, PerceptionResults);
		}
		else
		{
			Perception->GetFilteredActors([](const FActorPerceptionInfo& Info)
			{
				return !Info.bIsHostile;
			}, PerceptionResults);
		}
		for (const auto Actor : PerceptionResults)
		{
			if (!USussUtility::ActorHasAnyTags(Actor, IgnoreTags))
			{
				OutResults.Add(Actor);
			}
		}
	}
}

FSussActorPerceptionInfo::FSussActorPerceptionInfo(const FActorPerceptionInfo& Info): bIsSeen(0),
	bIsHeard(0),
	bIsHostile(Info.bIsHostile)
{
	Target = Info.Target;
	const FAISenseID SightID = GetDefault<UAISense_Sight>()->GetSenseID();
	const FAISenseID HearingID = GetDefault<UAISense_Hearing>()->GetSenseID();
	bIsSeen = Info.HasKnownStimulusOfSense(SightID);
	bIsHeard = Info.HasKnownStimulusOfSense(HearingID);
	LastLocation = Info.GetLastStimulusLocation();
	LastSensedStimuli = Info.LastSensedStimuli;
}

FString FSussActorPerceptionInfo::ToString() const
{
	return FString::Printf(TEXT("Target: %s  Seen: %d  Heard: %d At: %s"),
		Target.IsValid() ? *Target->GetActorNameOrLabel() : TEXT("null"),
		bIsSeen, bIsHeard, *LastLocation.ToString());
}

USussPerceptionKnownHostilesExtendedQueryProvider::USussPerceptionKnownHostilesExtendedQueryProvider()
{
	QueryTag = TAG_SussQueryPerceptionKnownHostilesExtended;
	QueryValueName = SUSS::PerceptionInfoValueName;
	QueryValueType = ESussContextValueType::Struct;
}

TSubclassOf<UAISense> USussPerceptionKnownHostilesExtendedQueryProvider::GetSenseClass(
	const TMap<FName, FSussParameter>& Params)
{
	return USussUtility::GetSenseClassFromParams(Params);
}

void USussPerceptionKnownHostilesExtendedQueryProvider::GetIgnoreTags(
	const TMap<FName, FSussParameter>& Params,
	FGameplayTagContainer& OutTags)
{
	USussUtility::GetIgnoreTagsFromParams(Params, OutTags);
}

void USussPerceptionKnownHostilesExtendedQueryProvider::ExecuteQuery(USussBrainComponent* Brain,
                                                                     AActor* Self,
                                                                     const TMap<FName, FSussParameter>& Params,
                                                                     const FSussContext& Context,
                                                                     TArray<FSussContextValue>& OutResults)
{
	if (const auto Perception = Brain->GetPerceptionComponent())
	{
		TSubclassOf<UAISense> SenseClass = GetSenseClass(Params);
		const FAISenseID SenseID = UAISense::GetSenseID(SenseClass);
		FGameplayTagContainer IgnoreTags;
		GetIgnoreTags(Params, IgnoreTags);
		for (auto It = Perception->GetPerceptualDataConstIterator(); It; ++It)
		{
			const FActorPerceptionInfo& Info = It->Value;

			if (Info.bIsHostile && Info.HasAnyKnownStimulus())
			{
				if (!SenseClass || Info.HasKnownStimulusOfSense(SenseID))
				{
					if (!USussUtility::ActorHasAnyTags(Info.Target.Get(), IgnoreTags))
					{
						OutResults.Add(FSussContextValue(MakeShared<FSussActorPerceptionInfo>(Info)));
					}
				}
			}
		}
		
	}
}
