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
		RegisterTestQueryProviders(WorldFixture->GetWorld());
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
			Brain->GenerateContexts(Self, Action, Contexts);

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
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 1))
			{
				TestEqual("Self reference", Contexts[0].Self, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector(10, -20, 50));
				TestEqual("Rotation", Contexts[0].Rotation, FRotator::ZeroRotator);
			}
		});

		It("One location dimension, multiple items", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query multiple locations
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery {TAG_TestMultipleLocationQuery });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 3))
			{
				TestEqual("Self reference 0", Contexts[0].Self, Self);
				TestNull("Target 0", Contexts[0].Target.Get());
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				TestEqual("Rotation 0", Contexts[0].Rotation, FRotator::ZeroRotator);

				TestEqual("Self reference 1", Contexts[1].Self, Self);
				TestNull("Target 1", Contexts[1].Target.Get());
				TestEqual("Location 1", Contexts[1].Location, FVector(20, 100, -2));
				TestEqual("Rotation 1", Contexts[1].Rotation, FRotator::ZeroRotator);

				TestEqual("Self reference 2", Contexts[2].Self, Self);
				TestNull("Target 2", Contexts[2].Target.Get());
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));
				TestEqual("Rotation 2", Contexts[2].Rotation, FRotator::ZeroRotator);

			}
		});

		It("Two dimensions, multiple items", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query multiple locations
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery {TAG_TestMultipleLocationQuery });
			Action.Queries.Add(FSussQuery {TAG_TestMultipleRotationQuery });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 9))
			{
				TestEqual("Self reference 0", Contexts[0].Self, Self);
				TestNull("Target 0", Contexts[0].Target.Get());
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				TestEqual("Rotation 0", Contexts[0].Rotation, FRotator(10, -20, 50));

				TestEqual("Self reference 1", Contexts[1].Self, Self);
				TestNull("Target 1", Contexts[1].Target.Get());
				TestEqual("Location 1", Contexts[1].Location, FVector(20, 100, -2));
				TestEqual("Rotation 1", Contexts[1].Rotation, FRotator(10, -20, 50));

				TestEqual("Self reference 2", Contexts[2].Self, Self);
				TestNull("Target 2", Contexts[2].Target.Get());
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));
				TestEqual("Rotation 2", Contexts[2].Rotation, FRotator(10, -20, 50));

				TestEqual("Self reference3", Contexts[3].Self, Self);
				TestNull("Target3", Contexts[3].Target.Get());
				TestEqual("Location3", Contexts[3].Location, FVector(10, -20, 50));
				TestEqual("Rotation3", Contexts[3].Rotation, FRotator(20, 100, -2));

				TestEqual("Self reference 4", Contexts[4].Self, Self);
				TestNull("Target 4", Contexts[4].Target.Get());
				TestEqual("Location 4", Contexts[4].Location, FVector(20, 100, -2));
				TestEqual("Rotation 4", Contexts[4].Rotation, FRotator(20, 100, -2));

				TestEqual("Self reference 5", Contexts[5].Self, Self);
				TestNull("Target 5", Contexts[5].Target.Get());
				TestEqual("Location 5", Contexts[5].Location, FVector(-40, 220, 750));
				TestEqual("Rotation 5", Contexts[5].Rotation, FRotator(20, 100, -2));
	
				TestEqual("Self reference 6", Contexts[6].Self, Self);
				TestNull("Target 6", Contexts[6].Target.Get());
				TestEqual("Location 6", Contexts[6].Location, FVector(10, -20, 50));
				TestEqual("Rotation 6", Contexts[6].Rotation, FRotator(-40, 220, 750));

				TestEqual("Self reference 7", Contexts[7].Self, Self);
				TestNull("Target 7", Contexts[7].Target.Get());
				TestEqual("Location 7", Contexts[7].Location, FVector(20, 100, -2));
				TestEqual("Rotation 7", Contexts[7].Rotation, FRotator(-40, 220, 750));

				TestEqual("Self reference 8", Contexts[8].Self, Self);
				TestNull("Target 8", Contexts[8].Target.Get());
				TestEqual("Location 8", Contexts[8].Location, FVector(-40, 220, 750));
				TestEqual("Rotation 8", Contexts[8].Rotation, FRotator(-40, 220, 750));
			}
		});
	});
}

UE_ENABLE_OPTIMIZATION

#endif
