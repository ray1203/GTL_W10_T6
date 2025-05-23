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

    virtual void SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InBlendDuration = 1.f, float InPlayRate = 1.f) override;
    UAnimSequence* GetAnimationSequence() { return AnimSequence; }
    virtual void SetPlaying(bool bInPlaying) override;
    void SetLooping(bool bInLooping) override;

    virtual FPoseContext& GetOutput() override;

    // CurrentTime
    FORCEINLINE float GetCurrentTime() const { return CurrentTime; }
    FORCEINLINE void  SetCurrentTime(float InTime) { CurrentTime = InTime; }

    // Is Playing
    FORCEINLINE bool  IsPlaying() const { return bIsPlaying; }
    FORCEINLINE void  SetPlayingState(bool bInPlay) { bIsPlaying = bInPlay; }

    // Play Rate
    FORCEINLINE float GetPlayRate() const { return PlayRate; }
    FORCEINLINE void  SetPlayRate(float InRate) { PlayRate = InRate; }

    // Is Looping
    FORCEINLINE bool  IsLooping() const { return bIsLooping; }
    // SetLooping은 이미 있음

    // 크로스페이드 블렌딩 
    void StartCrossfade(UAnimSequence* NewTargetSequence, bool bTargetLooping, float InBlendDuration,  float InTargetPlayRate = 1.f);
    bool IsBlending() const { return bIsBlending; }

    UAnimSequence* GetCurrentAnimSequnece() const { return AnimSequence; } // 현재 '주' 애니메이션

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

    // 크로스페이드 블렌딩용 내부 변수
    UAnimSequence* TargetAnimSequence = nullptr; // 블렌딩 대상 애니메이션
    float TargetCurrentTime = 0.f; // 타겟 애니메이션의 현재 시간
    float TargetPlayRate = 1.f; // 타겟 애니메이션의 재생 속도
    bool bTargetLooping; // 타겟 애니메이션의 루프 여부

    bool bIsBlending = false; // 현재 블렌딩 중인지 여부
    float BlendTimeElapsed = 0.f; // 블렌딩 경과 시간
    float BlendDurationTotal = 0.f; // 총 블렌딩 시간

};

