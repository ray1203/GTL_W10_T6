#pragma once
#include "Animation/AnimInstance.h"

class UAnimBlendLinearNodeInstance : public UAnimInstance
{
    DECLARE_CLASS(UAnimBlendLinearNodeInstance, UObject)
public:
    UAnimBlendLinearNodeInstance();
    ~UAnimBlendLinearNodeInstance() = default;

    void SetPoseSourceA(UAnimInstance* InPoseSourceA) { PoseSourceA = InPoseSourceA; }
    void SetPoseSourceB(UAnimInstance* InPoseSourceB) { PoseSourceB = InPoseSourceB; }
    void SetBlendAlpha(float InBlendAlpha) { BlendAlpha = InBlendAlpha; }

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    void ResetOutputToRefPose();
private:
    UAnimInstance* PoseSourceA = nullptr;
    UAnimInstance* PoseSourceB = nullptr;
    float BlendAlpha = 0.0f;
};

