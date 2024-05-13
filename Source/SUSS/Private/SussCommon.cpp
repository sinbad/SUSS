#include "SussCommon.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussActionParentTag, "Suss.Action", "Parent tag of all SUSS Actions (you should add subtags to this)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussInputParentTag, "Suss.Input", "Parent tag of all SUSS Inputs (you should add subtags to this)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussQueryParentTag, "Suss.Query", "Parent tag of all SUSS Querys (you should add subtags to this)")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SussParamParentTag, "Suss.Param", "Parent tag of all SUSS automatic params (you should add subtags to this)")

const FName SUSS::KeyParamName("Key");
const FName SUSS::TagParamName("Tag");
const FName SUSS::AllowRemoteParamName("AllowRemote");
const FName SUSS::CompletionDelayParamName("CompletionDelay");
const FName SUSS::WaitForEndParamName("WaitForEnd");
const FName SUSS::RadiusParamName("Radius");
const FName SUSS::PerceptionInfoValueName("PerceptionInfo");
