#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Public/Animation/AnimTypes.h"
#include "Engine/Source/Runtime/Core/Misc/FrameRate.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"

struct FAnimationCurveData
{

};

class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)

public:
    UAnimDataModel();
    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;

private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;

    
};
