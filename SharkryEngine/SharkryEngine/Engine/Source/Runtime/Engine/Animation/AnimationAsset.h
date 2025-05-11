#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimTypes.h"

class UAnimDataModel;

class UAnimationAsset : public UObject
{
    DECLARE_CLASS(UAnimationAsset, UObject)

public:
    UAnimationAsset();
    ~UAnimationAsset() = default;

    virtual void GetAnimationPose(FPoseContext& PoseContext, const FAnimExtractContext& ExtractContext) const;

    virtual FString GetName() const;
    virtual float GetPlayLength() const;
    virtual FFrameRate GetFrameRate() const;
    virtual int32 GetNumberOfFrames() const;
    virtual int32 GetNumberOfKeys() const;
    virtual const FAnimationCurveData& GetCurveData() const;
    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;


    virtual UAnimDataModel* GetDataModel() const;
    virtual void SetDataModel(UAnimDataModel* InDataModel);

protected:
    UAnimDataModel* DataModel = nullptr;
};

