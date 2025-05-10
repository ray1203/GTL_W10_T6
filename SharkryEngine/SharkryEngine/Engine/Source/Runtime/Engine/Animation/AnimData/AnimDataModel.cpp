#include "AnimDataModel.h"

UAnimDataModel::UAnimDataModel()
{
}

const TArray<FBoneAnimationTrack>& UAnimDataModel::GetBoneAnimationTracks() const
{
    // TODO: 여기에 return 문을 삽입합니다.
    return BoneAnimationTracks;
}
