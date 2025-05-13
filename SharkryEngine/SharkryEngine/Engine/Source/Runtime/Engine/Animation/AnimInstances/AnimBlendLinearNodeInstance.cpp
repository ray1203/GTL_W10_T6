#include "AnimBlendLinearNodeInstance.h"
#include "Animation/AnimationRuntimeUtils.h"
UAnimBlendLinearNodeInstance::UAnimBlendLinearNodeInstance()
{
    Output.AnimInstance = this;
    Output.bIsAdditivePose = false;
}

void UAnimBlendLinearNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Output.AnimInstance = this;

    UAnimInstance* SourceA = PoseSourceA;
    UAnimInstance* SourceB = PoseSourceB;

    if (SourceA && SourceB)
    {
        SourceA->UpdateAnimation(DeltaSeconds);
        SourceB->UpdateAnimation(DeltaSeconds);

        const FPoseContext& ContextA = SourceA->GetOutput();
        const FPoseContext& ContextB = SourceB->GetOutput();

        if (ContextA.AnimInstance || ContextB.AnimInstance ||
            ContextA.Pose.BoneTransforms.IsEmpty() || ContextB.Pose.BoneTransforms.IsEmpty())
        {
            ResetOutputToRefPose();
        }

        FAnimationRuntime::BlendTwoCompactPoses(ContextA.Pose, ContextB.Pose, this->BlendAlpha, Output.Pose);
        Output.bIsAdditivePose = false;
    }
    else
    {
        // PoseSourceA 또는 PoseSourceB가 nullptr인 경우, 기본 포즈로 초기화
        ResetOutputToRefPose();
    }
}

void UAnimBlendLinearNodeInstance::ResetOutputToRefPose()
{
    if (GetSkeletalMesh() != nullptr && GetSkeletalMesh()->Skeleton != nullptr)
    {
        Output.ResetToRefPose(GetRequiredBoneLocalTransforms());
    }
    else
    {
        Output.Pose.BoneTransforms.Empty();
    }
    Output.AnimInstance = this;
    Output.bIsAdditivePose = false; 
}
