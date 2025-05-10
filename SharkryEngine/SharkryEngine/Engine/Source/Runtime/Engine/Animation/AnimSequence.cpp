#include "AnimSequence.h"
#include "AnimData/AnimDataModel.h"

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
        SamplePose(
            Tracks[i].InternalTrack,
            PoseContext.Pose.BoneMatrices[i],
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
