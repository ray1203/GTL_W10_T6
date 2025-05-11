#pragma once
#include "Animation/AnimInstance.h"

class UAnimationAsset;

class UAnimSingleNodeInstance : public UAnimInstance
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance)

public:
    UAnimSingleNodeInstance();
    ~UAnimSingleNodeInstance() = default;

    virtual void NativeInitializeAnimation() override;
    void SetAnimationAsset(UAnimationAsset* NewAsset, bool bIsLooping, float InPlayRate=1.f);
    void SetPlaying(bool bInPlaying);
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    FPoseContext& GetOutputPose() { return Output; }

protected:
    float CurrentPosition = 0.f;
    bool  bPlaying = false;
    bool  bLooping = false;
    float PlayRate = 1.f;
    UAnimationAsset* AnimAsset = nullptr;

    FPoseContext Output{ RequiredBones };
};

