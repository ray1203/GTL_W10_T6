#include "AnimSingleNodeInstance.h"
#include "Animation/AnimationStateMachine.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Math/JungleMath.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{

}

void UAnimSingleNodeInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    CurrentTime = 0.0f;
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!bIsPlaying || !AnimSequence) return;

    // 재생 속도(PlayRate)를 곱해서야 제대로 속도 조절이 됩니다.
    CurrentTime += DeltaSeconds * PlayRate;

    if (bIsLooping)
    {
        // 시퀀스 길이를 넘어가면 맨 앞으로 되돌리기
        CurrentTime = FMath::Fmod(CurrentTime, AnimSequence->GetPlayLength());
    }
    else
    {
        // 한 번만 재생할 땐 끝 시간을 넘지 않도록 고정
        CurrentTime = FMath::Clamp(CurrentTime, 0.f, AnimSequence->GetPlayLength() - KINDA_SMALL_NUMBER);
    }

    FPoseContext Pose(this);

    FAnimExtractContext Extract(CurrentTime, false);

    AnimSequence->GetAnimationPose(Pose, Extract);

    Output.Pose = Pose.Pose;
    // Output.Curve = Pose.Curve;
}

void UAnimSingleNodeInstance::UpdateNotify(float DeltaSeconds)
{
    if (!AnimSequence) return;

    TArray<FAnimNotifyEvent*> CurFrameNotifies;

    // End -> Begin -> Tick 순으로 실행될것임
    TArray<FAnimNotifyEvent*> NotifyBeginEvent;
    TArray<FAnimNotifyEvent*> NotifyTickEvent;
    TArray<FAnimNotifyEvent*> NotifyEndEvent;

    AnimSequence->GetAnimNotifies(CurrentTime, DeltaSeconds, bIsLooping, CurFrameNotifies);

    for (FAnimNotifyEvent* Notify : CurFrameNotifies) 
    {
        if (Notify->NotifyMode == ENotifyMode::Single) 
        {
            SkeletalMeshComp->HandleAnimNotify(*Notify);
            continue;
        }

        // 현재에는 있는데 예전에는 없으면 -> Begin
        if (!PrevFrameNotifies.Contains(Notify)) 
        {
            NotifyBeginEvent.Add(Notify);
            continue;
        }
        else 
        {
            // 현재와 예전 둘 다 있으면 Tick
            NotifyTickEvent.Add(Notify);

            // Prev에서 해당 제거해서 이후 EndEvent 찾기 쉽도록 하기
            PrevFrameNotifies.Remove(Notify);
            continue;
        }
    }

    // 앞선 과정에서 Tick은 다 제거 했고 Begin은 Prev에 추가되지 않았으므로
    // PrevFrameNotifies에 남은 것중 Single이 아닌 것은 전부 End 이벤트

    for (FAnimNotifyEvent* Notify : PrevFrameNotifies)
    {
        if (Notify->NotifyMode == ENotifyMode::State)
        {
            NotifyEndEvent.Add(Notify);
        }
    }

    //Prev를 현재 FrameNotifies로 업데이트
    PrevFrameNotifies.Empty();
    for (FAnimNotifyEvent* Notify : CurFrameNotifies) 
    {
        PrevFrameNotifies.Add(Notify);
    }

    // End -> Begin -> Tick 순으로 실행
    for (FAnimNotifyEvent* Notify : NotifyEndEvent)
    {
        Notify->NotifyState = ENotifyState::End;
        SkeletalMeshComp->HandleAnimNotify(*Notify);
    }

    for (FAnimNotifyEvent* Notify : NotifyBeginEvent)
    {
        Notify->NotifyState = ENotifyState::Begin;
        SkeletalMeshComp->HandleAnimNotify(*Notify);
    }

    for (FAnimNotifyEvent* Notify : NotifyTickEvent)
    {
        Notify->NotifyState = ENotifyState::Tick;
        SkeletalMeshComp->HandleAnimNotify(*Notify);
    }
}

void UAnimSingleNodeInstance::SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InBlendDuration, float InPlayRate)
{
    if (NewSequence == nullptr || NewSequence == AnimSequence)
    {
        return;
    }

    AnimSequence = NewSequence;
    PlayRate = InPlayRate;
    CurrentTime = 0.0f;
}

FPoseContext& UAnimSingleNodeInstance::GetOutput()
{
    return Output;
}

void UAnimSingleNodeInstance::SetPlaying(bool bInPlaying)
{
    bIsPlaying = bInPlaying;
}

void UAnimSingleNodeInstance::SetLooping(bool bInLooping)
{
    bIsLooping = bInLooping;
}
