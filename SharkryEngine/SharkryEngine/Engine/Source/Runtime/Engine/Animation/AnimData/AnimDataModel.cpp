#include "AnimDataModel.h"

UAnimDataModel::UAnimDataModel()
{
}

const TArray<FBoneAnimationTrack>& UAnimDataModel::GetBoneAnimationTracks() const
{
    return BoneAnimationTracks;
}

float UAnimDataModel::GetPlayLength() const
{
    return PlayLength;
}

FFrameRate UAnimDataModel::GetFrameRate() const
{
    return FrameRate;
}

int32 UAnimDataModel::GetNumberOfFrames() const
{
    return NumberOfFrames;
}

int32 UAnimDataModel::GetNumberOfKeys() const
{
    return NumberOfKeys;
}

const FAnimationCurveData& UAnimDataModel::GetCurveData() const
{
    return CurveData;
}
