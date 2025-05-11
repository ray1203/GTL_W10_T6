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


    FString GetName() const;
    float GetPlayLength() const;
    FFrameRate GetFrameRate() const;
    int32 GetNumberOfFrames() const;
    int32 GetNumberOfKeys() const;
    const FAnimationCurveData& GetCurveData() const;
    const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;
    const FBoneContainer& GetBoneContainer() const;


    void SetName(const FString& InName);
    void SetPlayLength(float InLength);
    void SetFrameRate(FFrameRate InFrameRate);
    void SetNumberOfFrames(int32 InNumFrames);
    void SetNumberOfKeys(int32 InNumKeys);
    void SetCurveData(const FAnimationCurveData& InCurveData);
    void SetBoneAnimationTracks(const TArray<FBoneAnimationTrack>& InTracks);
    void SetBoneContainer(const FBoneContainer& InBoneContainer);


private:
    FString Name;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    FBoneContainer BoneContainer;
};

