#include "SussParameterProvider.h"

FSussParameter USussParameterProvider::Evaluate_Implementation(const USussBrainComponent* Brain,
                                                               const FSussContext& Context,
                                                               const TMap<FName, FSussParameter>& Parameters) const
{
	return FSussParameter(0);
}
