#pragma once
#include "Animation/AnimInstance.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimSequence.h"

class UAnimationStateMachine;
class FAnimNotifyEvent;
class UMyAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UMyAnimInstance, UAnimInstance)

public:
    UMyAnimInstance();
    ~UMyAnimInstance() = default;
    virtual void NativeInitializeAnimation() override;
    virtual void UpdateNotify(float DeltaSeconds) override;

    void SetIdleAnimSequence(UAnimSequence* InIdleAnimSequence) { IdleAnimSequence = InIdleAnimSequence; }
    void SetWalkAnimSequence(UAnimSequence* InWalkAnimSequence) { WalkAnimSequence = InWalkAnimSequence; }
    void SetRunAnimSequence(UAnimSequence* InRunAnimSequence) { RunAnimSequence = InRunAnimSequence; }
    void SetJumpAnimSequence(UAnimSequence* InJumpAnimSequence) { JumpAnimSequence = InJumpAnimSequence; }
    UAnimSequence* GetIdleAnimSequence() { return IdleAnimSequence; }
    UAnimSequence* GetWalkAnimSequence() { return WalkAnimSequence; }
    UAnimSequence* GetRunAnimSequence() { return RunAnimSequence; }
    UAnimSequence* GetJumpAnimSequence() { return JumpAnimSequence; }

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    virtual void SetAnimationSequence(UAnimSequence* NewSequence, float InPlayRate = 1.f) override;
    UAnimSequence* GetAnimationSequence() { return AnimSequence; }
    virtual void SetPlaying(bool bInPlaying) override;
    void SetLooping(bool bInLooping) override;

    virtual FPoseContext& GetOutput() override;

private:
    UAnimSequence* AnimSequence = nullptr;
    FPoseContext Output;
    float CurrentTime = 0.0f;
    bool bIsPlaying = false;
    float PlayRate = 1.0f;
    bool bIsLooping = false;

    // 애니매이션 노티파이
    TArray<FAnimNotifyEvent*> PrevFrameNotifies;

    // 애니매이션 시퀀스
    UAnimSequence* IdleAnimSequence = nullptr;
    UAnimSequence* WalkAnimSequence = nullptr;
    UAnimSequence* RunAnimSequence = nullptr;
    UAnimSequence* JumpAnimSequence = nullptr;

    // 상태 머신
    UAnimationStateMachine* StateMachine = nullptr;

};

