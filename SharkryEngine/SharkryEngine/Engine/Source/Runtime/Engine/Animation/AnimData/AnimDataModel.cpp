#include "AnimDataModel.h"

UAnimDataModel::UAnimDataModel()
{
}

const TArray<FBoneAnimationTrack>& UAnimDataModel::GetBoneAnimationTracks() const
{
    return BoneAnimationTracks;
}

const FBoneContainer& UAnimDataModel::GetBoneContainer() const
{
    return BoneContainer;
}

FString UAnimDataModel::GetName() const
{
    return Name;
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

void UAnimDataModel::SetBoneAnimationTracks(const TArray<FBoneAnimationTrack>& InTracks)
{
    BoneAnimationTracks = InTracks;
}

void UAnimDataModel::SetBoneContainer(const FBoneContainer& InBoneContainer)
{
    BoneContainer = InBoneContainer;
}

void UAnimDataModel::SetName(const FString& InName)
{
    Name = InName;
}

void UAnimDataModel::SetPlayLength(float InLength)
{
    PlayLength = InLength;
}

void UAnimDataModel::SetFrameRate(FFrameRate InFrameRate)
{
    FrameRate = InFrameRate;
}

void UAnimDataModel::SetNumberOfFrames(int32 InNumFrames)
{
    NumberOfFrames = InNumFrames;
}

void UAnimDataModel::SetNumberOfKeys(int32 InNumKeys)
{
    NumberOfKeys = InNumKeys;
}

void UAnimDataModel::SetCurveData(const FAnimationCurveData& InCurveData)
{
    CurveData = InCurveData;
}
