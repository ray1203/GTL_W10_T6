#pragma once
#include "Animation/AnimInstance.h"

class FAnimNotifyEvent;
class UAnimSequence;
class UAnimSingleNodeInstance : public UAnimInstance 
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance)

public:
    UAnimSingleNodeInstance();
    ~UAnimSingleNodeInstance() = default;

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void UpdateNotify(float DeltaSeconds) override;
    virtual void SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InPlayRate = 1.f) override;
    UAnimSequence* GetAnimationSequence() { return AnimSequence; }
    virtual FPoseContext& GetOutput() override;
    virtual void SetPlaying(bool bInPlaying) override;

protected:
    UAnimSequence* AnimSequence = nullptr;
    FPoseContext Output;
    float CurrentTime = 0.0f;
    bool bIsPlaying = true;
    float PlayRate = 1.0f;
    bool bIsLooping = true;

    TArray<FAnimNotifyEvent*> PrevFrameNotifies;
};
