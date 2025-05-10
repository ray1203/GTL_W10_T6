#include "AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/Skeleton.h"

UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
}

TMap<FName, uint32> UAnimInstance::GetRequiredBoneNames()
{
    return SkeletalMesh->Skeleton->BoneNameToIndex;
}

TArray<FMatrix> UAnimInstance::GetRequiredBoneLocalTransforms()
{
    TArray<FMatrix> BoneLocalTransforms;

    for (const FBoneNode& Node : SkeletalMesh->Skeleton->BoneTree) 
    {
        BoneLocalTransforms.Add(Node.BindTransform);
    }

    return BoneLocalTransforms;
}

void UAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    //PreUpdateAnimation(Δt)
    NativeUpdateAnimation(DeltaSeconds);
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
}

void UAnimInstance::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    SkeletalMesh = InSkeletalMesh;
}
