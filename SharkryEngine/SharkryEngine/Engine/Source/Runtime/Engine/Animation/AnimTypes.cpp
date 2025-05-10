#include "AnimTypes.h"
#include "AnimInstance.h"

FPoseContext::FPoseContext(const FBoneContainer& InBoneContainer)
    : Pose(InBoneContainer.GetNumBones())
    , Curve(InBoneContainer.GetNumCurves())
    , BoneContainer(InBoneContainer)
{
}

FPoseContext::FPoseContext(const UAnimInstance* AnimInst)
    : Pose(AnimInst->GetRequiredBones().GetNumBones())
    , Curve(AnimInst->GetRequiredBones().GetNumCurves())
    , BoneContainer(AnimInst->GetRequiredBones())
{
}
