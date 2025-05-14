#include "AnimInstance.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimSequence.h"

UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
}

void UAnimInstance::NativeInitializeAnimation()
{
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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
    UpdateNotify(DeltaSeconds);
}

void UAnimInstance::UpdateNotify(float DeltaSeconds)
{
}

void UAnimInstance::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    SkeletalMesh = InSkeletalMesh;
}

void UAnimInstance::SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComp)
{
    SkeletalMeshComp = InSkeletalMeshComp;
}

FPoseContext& UAnimInstance::GetOutput()
{
    return Output;
}

void UAnimInstance::SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InBlendDuration, float InPlayRate)
{
}

void UAnimInstance::SetPlaying(bool bInPlaying)
{
}

void UAnimInstance::StartCrossfade(UAnimSequence* NewTargetSequence, bool bTargetLooping, float InBlendDuration, float InTargetPlayRate)
{
}


void UAnimInstance::SetLooping(bool bInLooping)
{
}
