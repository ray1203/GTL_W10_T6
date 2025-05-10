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

    const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;
    float GetPlayLength() const;
    FFrameRate GetFrameRate() const;
    int32 GetNumberOfFrames() const;
    int32 GetNumberOfKeys() const;
    const FAnimationCurveData& GetCurveData() const;;

private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;
};

