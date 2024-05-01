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
		UnregisterTestQueryProviders(WorldFixture->GetWorld());
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
				TestEqual("Self reference", Contexts[0].ControlledActor, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector::ZeroVector);
			}
		});

		It("One location dimension, single item", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query single item
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery { FGameplayTag::RequestGameplayTag(USussTestSingleLocationQueryProvider::TagName) });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 1))
			{
				TestEqual("Self reference", Contexts[0].ControlledActor, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector(10, -20, 50));
			}
		});

		It("One location dimension, multiple items", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query multiple locations
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery { FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName) });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 3))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				TestNull("Target 0", Contexts[0].Target.Get());
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				TestNull("Target 1", Contexts[1].Target.Get());
				TestEqual("Location 1", Contexts[1].Location, FVector(20, 100, -2));

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				TestNull("Target 2", Contexts[2].Target.Get());
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));

			}
		});

		It("Zero results from one dimension", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			// Query multiple locations, and zero targets. Result should be nothing even though there are locations, because Nx0 = 0
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery { FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName) });
			Action.Queries.Add(FSussQuery { FGameplayTag::RequestGameplayTag(USussTestZeroTargetsQueryProvider::TagName) });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			TestEqual("Number of contexts", Contexts.Num(), 0);
		});

		It("Named params combined with locations", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(
				Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			FSussActionDef Action;
			Action.Queries.Add(FSussQuery {FGameplayTag::RequestGameplayTag(USussTestNamedFloatValueQueryProvider::TagName) }); // 2 items
			Action.Queries.Add(FSussQuery {FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName) }); // 3 items
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 6))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				if (TestTrue("Named Range 0",Contexts[0].NamedValues.Contains("Range")))
				{
					TestEqual("Named 0", Contexts[0].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				TestEqual("Location 1", Contexts[1].Location, FVector(10, -20, 50));
				if (TestTrue("Named Range 1",Contexts[1].NamedValues.Contains("Range")))
				{
					TestEqual("Named 1", Contexts[1].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				TestEqual("Location 2", Contexts[2].Location, FVector(20, 100, -2));
				if (TestTrue("Named Range 2",Contexts[2].NamedValues.Contains("Range")))
				{
					TestEqual("Named 2", Contexts[2].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 3", Contexts[3].ControlledActor, Self);
				TestEqual("Location 3", Contexts[3].Location, FVector(20, 100, -2));
				if (TestTrue("Named Range 3",Contexts[3].NamedValues.Contains("Range")))
				{
					TestEqual("Named 3", Contexts[3].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}

				TestEqual("Self reference 4", Contexts[4].ControlledActor, Self);
				TestEqual("Location 4", Contexts[4].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Range 4",Contexts[4].NamedValues.Contains("Range")))
				{
					TestEqual("Named 4", Contexts[4].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 5", Contexts[5].ControlledActor, Self);
				TestEqual("Location 5", Contexts[5].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Range 5",Contexts[5].NamedValues.Contains("Range")))
				{
					TestEqual("Named 5", Contexts[5].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}
			}
		});

		It("Named params x2",
		   [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(
				Self->AddComponentByClass(USussBrainComponent::StaticClass(),
				                          false,
				                          FTransform::Identity,
				                          false));

			FSussActionDef Action;
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestNamedFloatValueQueryProvider::TagName)
			}); // 2 items
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestNamedLocationValueQueryProvider::TagName)
			}); // 3 items
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 6))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				if (TestTrue("Named MapRef 0", Contexts[0].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 0",
					          Contexts[0].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(120, -450, 80));
				}
				if (TestTrue("Named Range 0", Contexts[0].NamedValues.Contains("Range")))
				{
					TestEqual("Named Range 0", Contexts[0].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				if (TestTrue("Named MapRef 1", Contexts[1].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 1",
					          Contexts[1].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(120, -450, 80));
				}
				if (TestTrue("Named Range 1", Contexts[1].NamedValues.Contains("Range")))
				{
					TestEqual("Named 1", Contexts[1].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				if (TestTrue("Named MapRef 2", Contexts[2].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 2",
					          Contexts[2].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(70, 123, -210));
				}
				if (TestTrue("Named Range 2", Contexts[2].NamedValues.Contains("Range")))
				{
					TestEqual("Named 2", Contexts[2].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 3", Contexts[3].ControlledActor, Self);
				if (TestTrue("Named MapRef 3", Contexts[3].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 3",
					          Contexts[3].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(70, 123, -210));
				}
				if (TestTrue("Named Range 3", Contexts[3].NamedValues.Contains("Range")))
				{
					TestEqual("Named 3", Contexts[3].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}

				TestEqual("Self reference 4", Contexts[4].ControlledActor, Self);
				if (TestTrue("Named MapRef 4", Contexts[4].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 4",
					          Contexts[4].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(-35, 65, 0));
				}
				if (TestTrue("Named Range 4", Contexts[4].NamedValues.Contains("Range")))
				{
					TestEqual("Named 4", Contexts[4].NamedValues["Range"].Value.Get<float>(), 2000.0f);
				}

				TestEqual("Self reference 5", Contexts[5].ControlledActor, Self);
				if (TestTrue("Named MapRef 5", Contexts[5].NamedValues.Contains("MapRef")))
				{
					TestEqual("Named MapRef 5",
					          Contexts[5].NamedValues["MapRef"].Value.Get<FVector>(),
					          FVector(-35, 65, 0));
				}
				if (TestTrue("Named Range 5", Contexts[5].NamedValues.Contains("Range")))
				{
					TestEqual("Named 5", Contexts[5].NamedValues["Range"].Value.Get<float>(), 5000.0f);
				}
			}
		});
		
		It("Named struct params - shared pointers",
		   [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(
				Self->AddComponentByClass(USussBrainComponent::StaticClass(),
				                          false,
				                          FTransform::Identity,
				                          false));

			FSussActionDef Action;
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName)
			}); // 2 items
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestNamedStructSharedValueQueryProvider::TagName)
			}); // 3 items
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 6))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				if (TestTrue("Named Struct 0", Contexts[0].NamedValues.Contains("Struct")))
				{
					const auto S = Contexts[0].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 0", S))
					{
						const auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 0 int", TS->IntValue , 200);
						TestEqual("Named Struct 0 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				TestEqual("Location 1", Contexts[1].Location, FVector(20, 100, -2));
				if (TestTrue("Named Struct 1", Contexts[1].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[1].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 1", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 1 int", TS->IntValue , 200);
						TestEqual("Named Struct 1 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Struct 2", Contexts[2].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[2].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 2", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 2 int", TS->IntValue , 200);
						TestEqual("Named Struct 2 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 3", Contexts[3].ControlledActor, Self);
				TestEqual("Location 3", Contexts[3].Location, FVector(10, -20, 50));
				if (TestTrue("Named Struct 3", Contexts[3].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[3].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 3", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 3 int", TS->IntValue , -30);
						TestEqual("Named Struct 3 float", TS->FloatValue , 785.2f);
					}
				}

				TestEqual("Self reference 4", Contexts[4].ControlledActor, Self);
				TestEqual("Location 4", Contexts[4].Location, FVector(20, 100, -2));
				if (TestTrue("Named Struct 4", Contexts[4].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[4].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 4", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 4 int", TS->IntValue , -30);
						TestEqual("Named Struct 4 float", TS->FloatValue , 785.2f);
					}
				}

				TestEqual("Self reference 5", Contexts[5].ControlledActor, Self);
				TestEqual("Location 5", Contexts[5].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Struct 5", Contexts[5].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[5].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 5", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 5 int", TS->IntValue , -30);
						TestEqual("Named Struct 5 float", TS->FloatValue , 785.2f);

					}
				}


			}
		});

		It("Named struct params - raw pointers",
		   [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(
				Self->AddComponentByClass(USussBrainComponent::StaticClass(),
				                          false,
				                          FTransform::Identity,
				                          false));

			FSussActionDef Action;
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName)
			}); // 2 items
			Action.Queries.Add(FSussQuery{
				FGameplayTag::RequestGameplayTag(USussTestNamedStructRawPointerQueryProvider::TagName)
			}); // 3 items
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			if (TestEqual("Number of contexts", Contexts.Num(), 6))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				if (TestTrue("Named Struct 0", Contexts[0].NamedValues.Contains("Struct")))
				{
					const auto S = Contexts[0].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 0", S))
					{
						const auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 0 int", TS->IntValue , 200);
						TestEqual("Named Struct 0 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				TestEqual("Location 1", Contexts[1].Location, FVector(20, 100, -2));
				if (TestTrue("Named Struct 1", Contexts[1].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[1].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 1", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 1 int", TS->IntValue , 200);
						TestEqual("Named Struct 1 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Struct 2", Contexts[2].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[2].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 2", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 2 int", TS->IntValue , 200);
						TestEqual("Named Struct 2 float", TS->FloatValue , 123.4f);
					}
				}

				TestEqual("Self reference 3", Contexts[3].ControlledActor, Self);
				TestEqual("Location 3", Contexts[3].Location, FVector(10, -20, 50));
				if (TestTrue("Named Struct 3", Contexts[3].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[3].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 3", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 3 int", TS->IntValue , -30);
						TestEqual("Named Struct 3 float", TS->FloatValue , 785.2f);
					}
				}

				TestEqual("Self reference 4", Contexts[4].ControlledActor, Self);
				TestEqual("Location 4", Contexts[4].Location, FVector(20, 100, -2));
				if (TestTrue("Named Struct 4", Contexts[4].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[4].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 4", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 4 int", TS->IntValue , -30);
						TestEqual("Named Struct 4 float", TS->FloatValue , 785.2f);
					}
				}

				TestEqual("Self reference 5", Contexts[5].ControlledActor, Self);
				TestEqual("Location 5", Contexts[5].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Struct 5", Contexts[5].NamedValues.Contains("Struct")))
				{
					auto S = Contexts[5].NamedValues["Struct"].GetStructValue();
					if (TestNotNull("Named Struct 5", S))
					{
						auto TS = static_cast<const FSussTestContextValueStruct*>(S);
						TestEqual("Named Struct 5 int", TS->IntValue , -30);
						TestEqual("Named Struct 5 float", TS->FloatValue , 785.2f);

					}
				}


			}
		});

		It("Correlated query: locations -> named floats", [this]()
		{
			AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(
				Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));

			FSussActionDef Action;
			// Ordering is important, locations first then correlated floats
			Action.Queries.Add(FSussQuery {FGameplayTag::RequestGameplayTag(USussTestMultipleLocationQueryProvider::TagName) });
			Action.Queries.Add(FSussQuery {FGameplayTag::RequestGameplayTag(USussTestCorrelatedNamedFloatValueQueryProvider::TagName) }); 
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);

			// There are 3 locations generated, then the correlated query generates:
			//  - Location 1 generates 1 context
			//  - Location 2 generates NO contexts (and gets removed since correlation requires at least 1 intersecting result)
			//  - Location 3 generates 3 contexts
			// So there should be 4 contexts in total
			if (TestEqual("Number of contexts", Contexts.Num(), 4))
			{
				TestEqual("Self reference 0", Contexts[0].ControlledActor, Self);
				TestEqual("Location 0", Contexts[0].Location, FVector(10, -20, 50));
				if (TestTrue("Named Distance 0",Contexts[0].NamedValues.Contains("Distance")))
				{
					TestEqual("Named 0", Contexts[0].NamedValues["Distance"].Value.Get<float>(), 10.0f);
				}

				TestEqual("Self reference 1", Contexts[1].ControlledActor, Self);
				TestEqual("Location 1", Contexts[1].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Distance 1",Contexts[1].NamedValues.Contains("Distance")))
				{
					TestEqual("Named 1", Contexts[1].NamedValues["Distance"].Value.Get<float>(), -40.0f);
				}

				TestEqual("Self reference 2", Contexts[2].ControlledActor, Self);
				TestEqual("Location 2", Contexts[2].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Distance 2",Contexts[2].NamedValues.Contains("Distance")))
				{
					TestEqual("Named 2", Contexts[2].NamedValues["Distance"].Value.Get<float>(), 220.0f);
				}

				TestEqual("Self reference 3", Contexts[3].ControlledActor, Self);
				TestEqual("Location 3", Contexts[3].Location, FVector(-40, 220, 750));
				if (TestTrue("Named Distance 3",Contexts[3].NamedValues.Contains("Distance")))
				{
					TestEqual("Named 3", Contexts[3].NamedValues["Distance"].Value.Get<float>(), 750.0f);
				}
				
			}
		});		

		It("Query caching works as intended", [this]()
		{
		   	AActor* Self = WorldFixture->GetWorld()->SpawnActor<AActor>();
			auto Brain = Cast<USussBrainComponent>(Self->AddComponentByClass(USussBrainComponent::StaticClass(), false, FTransform::Identity, false));
			USussTestSingleLocationQueryProvider* Q = USussTestSingleLocationQueryProvider::StaticClass()->GetDefaultObject<USussTestSingleLocationQueryProvider>();

			// reset this manually, query objects are reused
			Q->NumTimesRun = 0;

			// Query single item
			FSussActionDef Action;
			Action.Queries.Add(FSussQuery {FGameplayTag::RequestGameplayTag(USussTestSingleLocationQueryProvider::TagName) });
			TArray<FSussContext> Contexts;
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query run count", Q->NumTimesRun, 1);
			
			if (TestEqual("Number of contexts", Contexts.Num(), 1))
			{
				TestEqual("Self reference", Contexts[0].ControlledActor, Self);
				TestNull("Target", Contexts[0].Target.Get());
				TestEqual("Location", Contexts[0].Location, FVector(10, -20, 50));
			}
		    // Now repeat; result should be the same because no time has passed & params are the same
		    Contexts.Empty();
		    Brain->GenerateContexts(Self, Action, Contexts);
			// Should re-use results
			TestEqual("Query should have re-used results", Q->NumTimesRun, 1);

			// Now make some time pass, above the 0.5s default query timeout & return 10x data value
			// Queries are ticked through the suss subsystem
			GetSUSS(WorldFixture->GetWorld())->Tick(1);
			// Tick the brain for consistency
			Brain->TickComponent(1, LEVELTICK_All, nullptr);

			// Running the query again should make it go down the other path
			Contexts.Empty();
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query should have run again because of time", Q->NumTimesRun, 2);

			// Issue a query with different params
			Action.Queries[0].Params.Add("OverrideX", FSussParameter(30.0f));
			Action.Queries[0].Params.Add("OverrideY", FSussParameter(-40.0f));
			Contexts.Empty();
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query should have run again because of parameters", Q->NumTimesRun, 3);

			Contexts.Empty();
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query should have re-used", Q->NumTimesRun, 3);

			// Going back to empty params should re-use the older query
			Action.Queries[0].Params.Empty();
			Contexts.Empty();
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query should have re-used older query", Q->NumTimesRun, 3);

			// Test going back to new query again
			Action.Queries[0].Params.Add("OverrideX", FSussParameter(30.0f));
			Action.Queries[0].Params.Add("OverrideY", FSussParameter(-40.0f));
			Contexts.Empty();
			Brain->GenerateContexts(Self, Action, Contexts);
			TestEqual("Query should have been re-used, newer again", Q->NumTimesRun, 3);
			
			// Changing which "self" we refer to should invalidate, if relevant
			AActor* WrongSelf = WorldFixture->GetWorld()->SpawnActor<AActor>();

			Contexts.Empty();
			Brain->GenerateContexts(WrongSelf, Action, Contexts);
			TestEqual("Query should run, changed self", Q->NumTimesRun, 4);
			
		});

	});
}

UE_ENABLE_OPTIMIZATION

#endif
