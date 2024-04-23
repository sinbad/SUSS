#include "Queries/SussPerceptionQueries.h"

#include "SussBrainComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Team.h"
#include "Perception/AISense_Touch.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownTargets, "Suss.Query.Perception.Targets.AllKnown", "Query all targets known to this agent's perception system.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownHostiles, "Suss.Query.Perception.Targets.HostilesKnown", "Query all hostiles known to this agent's perception system.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryPerceptionKnownNonHostiles, "Suss.Query.Perception.Targets.NonHostilesKnown", "Query all non-hostiles known to this agent's perception system.")

static const FName SussQuerySenseParam("Sense");
static const FName SussQuerySenseSightValue("Sight");
static const FName SussQuerySenseHearingValue("Hearing");
static const FName SussQuerySenseDamageValue("Damage");
static const FName SussQuerySenseTouchValue("Touch");
static const FName SussQuerySenseTeamValue("Team");

TSubclassOf<UAISense> GetSenseClassFromParams(const TMap<FName, FSussParameter>& Params)
{
	if (auto pSense = Params.Find(SussQuerySenseParam))
	{
		if (*pSense == SussQuerySenseSightValue)
		{
			return UAISense_Sight::StaticClass();
		}
		else if (*pSense == SussQuerySenseHearingValue)
		{
			return UAISense_Hearing::StaticClass();
		}
		else if (*pSense == SussQuerySenseDamageValue)
		{
			return UAISense_Damage::StaticClass();
		}
		else if (*pSense == SussQuerySenseTouchValue)
		{
			return UAISense_Touch::StaticClass();
		}
		else if (*pSense == SussQuerySenseTeamValue)
		{
			return UAISense_Team::StaticClass();
		}
	}

	return nullptr;
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
		TSubclassOf<UAISense> SenseClass = GetSenseClassFromParams(Params);
		if (SenseClass)
		{
			// Note that the "known" excludes forgotten actors but includes actors which are not *currently* perceived but
			// still remembered
			Perception->GetKnownPerceivedActors(SenseClass, PerceptionResults);
		}
		else
		{
			// There's no "known" query for all senses? Odd
			Perception->GetFilteredActors([](const FActorPerceptionInfo& Info)
			{
				return Info.HasAnyKnownStimulus();
			}, PerceptionResults);
		}
		for (const auto Actor : PerceptionResults)
		{
			OutResults.Add(Actor);
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
		TSubclassOf<UAISense> SenseClass = GetSenseClassFromParams(Params);

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
			OutResults.Add(Actor);
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
		TSubclassOf<UAISense> SenseClass = GetSenseClassFromParams(Params);
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
			OutResults.Add(Actor);
		}
	}
}