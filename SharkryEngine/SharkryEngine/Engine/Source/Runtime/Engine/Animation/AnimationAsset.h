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

    // 애니메이션 자산의 길이를 반환합니다.
    virtual float GetPlayLength() const;
    // 애니메이션 자산의 프레임 레이트를 반환합니다.
    virtual FFrameRate GetFrameRate() const;
    // 애니메이션 자산의 프레임 수를 반환합니다.
    virtual int32 GetNumberOfFrames() const;
    // 애니메이션 자산의 키 수를 반환합니다.
    virtual int32 GetNumberOfKeys() const;
    // 애니메이션 자산의 커브 데이터를 반환합니다.
    virtual const FAnimationCurveData& GetCurveData() const;
    // 애니메이션 자산의 데이터 모델을 반환합니다.
    virtual UAnimDataModel* GetDataModel() const;

protected:
    UAnimDataModel* DataModel;
};

