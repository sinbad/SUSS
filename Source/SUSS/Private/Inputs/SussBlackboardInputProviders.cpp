
#include "Inputs/SussBlackboardInputProviders.h"

#include "SussBrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputBlackboardFloat, "Suss.Input.Blackboard.Float", "Get a float value from the blackboard, requires Name parameter 'Key'.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputBlackboardBool, "Suss.Input.Blackboard.Bool", "Get a boolean value from the blackboard, as 1 or 0 for True/False, requires Name parameter 'Key'.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputBlackboardAuto, "Suss.Input.Blackboard.Auto", "Get a value from the blackboard, automatically converting integers and booleans to a float value. Requires Name parameter 'Key'.")


USussBlackboardFloatInputProvider::USussBlackboardFloatInputProvider()
{
	InputTag = TAG_SussInputBlackboardFloat;
}

float USussBlackboardFloatInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	if (auto BB = Brain->GetBlackboard())
	{
		if (auto pParam = Parameters.Find(SUSS::KeyParamName))
		{
			return BB->GetValueAsFloat(pParam->NameValue);
		}
	}

	return 0;
}

USussBlackboardBoolInputProvider::USussBlackboardBoolInputProvider()
{
	InputTag = TAG_SussInputBlackboardBool;
}

float USussBlackboardBoolInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	if (auto BB = Brain->GetBlackboard())
	{
		if (auto pParam = Parameters.Find(SUSS::KeyParamName))
		{
			return BB->GetValueAsBool(pParam->NameValue) ? 1.0f : 0.0f;
		}
	}

	return 0;
}

USussBlackboardAutoInputProvider::USussBlackboardAutoInputProvider()
{
	InputTag = TAG_SussInputBlackboardAuto;
}

float USussBlackboardAutoInputProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
	const FSussContext& Context,
	const TMap<FName, FSussParameter>& Parameters) const
{
	if (auto BB = Brain->GetBlackboard())
	{
		if (auto pParam = Parameters.Find(SUSS::KeyParamName))
		{
			const FBlackboard::FKey KeyID = BB->GetKeyID(pParam->NameValue);
			auto KeyType = BB->GetKeyType(KeyID);
			if (KeyType == UBlackboardKeyType_Float::StaticClass())
			{
				return BB->GetValue<UBlackboardKeyType_Float>(KeyID);
			}
			if (KeyType == UBlackboardKeyType_Int::StaticClass())
			{
				return BB->GetValue<UBlackboardKeyType_Int>(KeyID);
			}
			if (KeyType == UBlackboardKeyType_Bool::StaticClass())
			{
				return BB->GetValue<UBlackboardKeyType_Bool>(KeyID) ? 1.0f : 0.0f;
			}
		}
	}

	return 0;
}
