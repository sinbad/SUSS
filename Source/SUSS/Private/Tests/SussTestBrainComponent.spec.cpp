#include "SussBrainComponent.h"
#include "SussGameSubsystem.h"
#include "SussTestQueryProviders.h"
#include "SussTestWorldFixture.h"
#if WITH_AUTOMATION_TESTS

UE_DISABLE_OPTIMIZATION


BEGIN_DEFINE_SPEC(FSussBrainTestContextsSpec,
				  "SUSS: Test  Contexts",
				  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

	TUniquePtr<FSussTestWorldFixture> WorldFixture;
END_DEFINE_SPEC(FSussBrainTestContextsSpec);


void FSussBrainTestContextsSpec::Define()
{
	BeforeEach([this]()
	{
		WorldFixture = MakeUnique<FSussTestWorldFixture>();
		if (auto SUSS = GetSUSS(WorldFixture->GetWorld()))
		{
			SUSS->RegisterQueryProvider(USussTestSingleLocationQueryProvider::StaticClass());
		}
	});
	AfterEach([this]()
	{
		WorldFixture.Reset();
	});
	
	Describe("FSussBrainTestContextsSpec", [this]()
	{
		It("Simple single entry, no dimensions", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Action with no queries
			FSussActionDef Action;
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 1))
			{
				TestEqual("Self reference", Contexts[0].Self, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector::ZeroVector);
				TestEqual("Rotation", Contexts[0].Rotation, FRotator::ZeroRotator);
			}
		});

		It("One location dimension, single item", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query single item
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery {TAG_TestSingleLocationQuery });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 1))
			{
				TestEqual("Self reference", Contexts[0].Self, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector(10, -20, 50));
				TestEqual("Rotation", Contexts[0].Rotation, FRotator::ZeroRotator);
			}
		});
	});
}

UE_ENABLE_OPTIMIZATION

#endif
