#include "AnimSequence.h"
#include "AnimData/AnimDataModel.h"
#include "AnimInstance.h"
#include "Components/Mesh/SkeletalMesh.h"

UAnimSequence::UAnimSequence()
{
}

void UAnimSequence::GetAnimationPose(FPoseContext& PoseContext, const FAnimExtractContext& ExtractContext) const
{
    const float CurrentTime = ExtractContext.CurrentTime;
    const FFrameRate FR = GetFrameRate();
    const auto& Tracks = DataModel->GetBoneAnimationTracks();

    for (int32 i = 0; i < Tracks.Num(); ++i)
    {
        int BoneIndex = PoseContext.AnimInstance->GetSkeletalMesh()->Skeleton->GetBoneIndex(Tracks[i].Name);
        if (BoneIndex == -1) continue;
        SamplePose(
            Tracks[i].InternalTrack,
            PoseContext.Pose.BoneMatrices[BoneIndex],
            CurrentTime,
            FR
        );
    }

    const auto& CurveData = DataModel->GetCurveData();
    for (int32 c = 0; c < CurveData.Channels.Num(); ++c)
    {
        PoseContext.Curve.CurveValues[c] =
            EvaluateCurveAtTime(CurveData.Channels[c], CurrentTime);
    }
}
