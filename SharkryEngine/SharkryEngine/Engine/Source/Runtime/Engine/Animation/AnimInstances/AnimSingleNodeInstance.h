#pragma once

#include "Engine/Animation/AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/AnimSequence.h"
#include "Animation/AnimationStateMachine.h"

class UAnimSingleNodeInstance : public UAnimInstance 
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance)

public:
    UAnimSingleNodeInstance();
    ~UAnimSingleNodeInstance() = default;

    void SetAnimSequence(UAnimSequence* InAnimSequence) { AnimSequence = InAnimSequence; }

    FPoseContext GetOutput() { return Output; }

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void UpdateNotify(float DeltaSeconds) override;

    UAnimationStateMachine* StateMachine = nullptr;

    EAnimState CurrentState = AS_Idle;
    EAnimState PreviousState = AS_Idle;
protected:
    UAnimSequence* AnimSequence = nullptr;
    FPoseContext Output;
    float CurrentTime = 0.0f;
    bool bPlaying = true;
    float PlayRate = 1.0f;
    bool bLooping = true;

    TArray<FAnimNotifyEvent*> PrevFrameNotifies;
};
