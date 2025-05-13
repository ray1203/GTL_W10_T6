#include "AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/Skeleton.h"
#include "Engine/Classes/Components/SkeletalMeshComponent.h"
#include "AnimTypes.h"


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
    // 구현은 UAnimSingleNodeInstance에서 처리
}

void UAnimInstance::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    SkeletalMesh = InSkeletalMesh;
}

void UAnimInstance::SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComp)
{
    SkeletalMeshComp = InSkeletalMeshComp;
}
