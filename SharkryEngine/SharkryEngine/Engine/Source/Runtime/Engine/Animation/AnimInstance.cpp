#include "AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/Skeleton.h"

UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
}

TMap<FName, uint32> UAnimInstance::GetRequiredBones()
{
    return SkeletalMesh->Skeleton->BoneNameToIndex;
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
}
