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
    UAnimSequence* GetAnimationSequence() { return AnimSequence; }
    void SetPlaying(bool bInPlaying);

    // 크로스페이드 블렌딩 
    void StartCrossfade(UAnimSequence* NewTargetSequence, float InBlendDuration, bool bTargetLooping, float InTargetPlayRate = 1.f);
    bool IsBlending() const { return bIsBlending; }

    UAnimSequence* GetCurrentAnimSequnece() const { return AnimSequence; } // 현재 '주' 애니메이션

    UAnimationStateMachine* StateMachine = nullptr;

    EAnimState CurrentState = AS_Idle;
    EAnimState PreviousState = AS_Idle;
protected:
    UAnimSequence* AnimSequence = nullptr;
    UAnimSequence* TargetSequence = nullptr; // 
    FPoseContext Output;
    float CurrentTime = 0.0f;
    bool bIsPlaying = true;
    float PlayRate = 1.0f;
    bool bIsLooping = true;

    TArray<FAnimNotifyEvent*> PrevFrameNotifies;

private:
    // 크로스페이드 블렌딩용 내부 변수
    UAnimSequence* TargetAnimSequence = nullptr; // 블렌딩 대상 애니메이션
    float TargetCurrentTime = 0.f; // 타겟 애니메이션의 현재 시간
    float TargetPlayRate = 1.f; // 타겟 애니메이션의 재생 속도
    bool bTargetLooping; // 타겟 애니메이션의 루프 여부

    bool bIsBlending = false; // 현재 블렌딩 중인지 여부
    float BlendTimeElapsed = 0.f; // 블렌딩 경과 시간
    float BlendDurationTotal = 0.f; // 총 블렌딩 시간

};
