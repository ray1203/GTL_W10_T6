#include "AnimationAsset.h"
#include "AnimData/AnimDataModel.h"

UAnimationAsset::UAnimationAsset()
{
}

void UAnimationAsset::GetAnimationPose(FPoseContext& PoseContext, const FAnimExtractContext& ExtractContext) const
{
}

FString UAnimationAsset::GetName() const
{
    return DataModel ? DataModel->GetName() : FString();
}

float UAnimationAsset::GetPlayLength() const
{
    return DataModel ? DataModel->GetPlayLength() : 0.f;
}

FFrameRate UAnimationAsset::GetFrameRate() const
{
    return DataModel ? DataModel->GetFrameRate() : FFrameRate();
}

int32 UAnimationAsset::GetNumberOfFrames() const
{
    return DataModel ? DataModel->GetNumberOfFrames() : 0;
}

int32 UAnimationAsset::GetNumberOfKeys() const
{
    return DataModel ? DataModel->GetNumberOfKeys() : 0;
}

const FAnimationCurveData& UAnimationAsset::GetCurveData() const
{
    if (DataModel)
    {
        return DataModel->GetCurveData();
    }

    return FAnimationCurveData();
}

const FBoneContainer& UAnimationAsset::GetBoneContainer() const
{
    if (DataModel)
    {
        return DataModel->GetBoneContainer();
    }

    return FBoneContainer();
}

const TArray<FBoneAnimationTrack>& UAnimationAsset::GetBoneAnimationTracks() const
{
    if (DataModel)
    {
        return DataModel->GetBoneAnimationTracks();
    }

    return TArray<FBoneAnimationTrack>();
}

UAnimDataModel* UAnimationAsset::GetDataModel() const
{
    return DataModel;
}

void UAnimationAsset::SetDataModel(UAnimDataModel* InDataModel)
{
    DataModel = InDataModel;
}
