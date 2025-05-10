#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimTypes.h"

class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)
    
public:
    UAnimDataModel();
    ~UAnimDataModel() = default;

    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;

private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;
};

