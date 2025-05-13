#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimTypes.h"

class FAnimNotifyEvent;
class UAnimSingleNodeInstance : public UAnimInstance 
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance)

public:
    UAnimSingleNodeInstance();
    ~UAnimSingleNodeInstance() = default;

    FPoseContext GetOutput() { return Output; }

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void UpdateNotify(float DeltaSeconds) override;
    void SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InPlayRate = 1.f);
    void SetPlaying(bool bInPlaying);

    UAnimationStateMachine* StateMachine = nullptr;

    EAnimState CurrentState = AS_Idle;
    EAnimState PreviousState = AS_Idle;
protected:
    UAnimSequence* AnimSequence = nullptr;
    FPoseContext Output;
    float CurrentTime = 0.0f;
    bool bIsPlaying = true;
    float PlayRate = 1.0f;
    bool bIsLooping = true;

    TArray<FAnimNotifyEvent*> PrevFrameNotifies;
};
