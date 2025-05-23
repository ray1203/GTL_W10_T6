#include "MyAnimInstance.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimNotify.h"
#include "Math/JungleMath.h"

UMyAnimInstance::UMyAnimInstance()
{
}

void UMyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 애니메이션 상태 머신 초기화
    if (StateMachine == nullptr)
    {
        StateMachine = FObjectFactory::ConstructObject<UAnimationStateMachine>(this);
        TArray<AActor*> ActorsCopy = GEngine->ActiveWorld->GetActiveLevel()->Actors;
        for (AActor* Actor : ActorsCopy)
        {
            if (Actor && Actor->IsA<APawn>())
            {
                StateMachine->SetPawn(Cast<APawn>(Actor));
                break;
            }
        }
    }
    // 애니메이션 시퀀스 초기화
    if (AnimSequence == nullptr)
    {
        AnimSequence = IdleAnimSequence;
    }
    CurrentTime = 0.0f;

    FName JumpStart = "JumpStart";
    FAnimNotifyEvent JumpStartEvent = FAnimNotifyEvent(0.5f, 0.0f, JumpStart);
    JumpAnimSequence->Notifies.Add(JumpStartEvent);
}

void UMyAnimInstance::UpdateNotify(float DeltaSeconds)
{
    Super::UpdateNotify(DeltaSeconds);

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

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!StateMachine) return;

    StateMachine->Update(DeltaSeconds);

    if (bIsBlending)
    {
        BlendTimeElapsed += DeltaSeconds;
        float BlendAlpha = FMath::Clamp(BlendTimeElapsed / BlendDurationTotal, 0.f, 1.f);

        // 현재 AnimSequence 애니메이션 시간 업데이트
        float PrevSourceTime = CurrentTime;
        if (AnimSequence)
        {
            CurrentTime += DeltaSeconds * PlayRate;
            float SourceDuration = AnimSequence->GetPlayLength();
            if (CurrentTime >= SourceDuration)
            {
                if (bIsLooping)
                {
                    CurrentTime = FMath::Fmod(CurrentTime, SourceDuration);
                }
                else
                {
                    CurrentTime = SourceDuration - KINDA_SMALL_NUMBER; // 마지막 프레임 유지 
                }
            }
        }

        // 타겟 애니메이션 시간 업데이트
        float PrevTargetTime = TargetCurrentTime;
        if (TargetAnimSequence)
        {
            TargetCurrentTime += DeltaSeconds * TargetPlayRate;
            float TargetDuration = TargetAnimSequence->GetPlayLength();
            if (TargetCurrentTime >= TargetDuration)
            {
                if (bTargetLooping)
                {
                    TargetCurrentTime = FMath::Fmod(TargetCurrentTime, TargetDuration);
                }
                else
                {
                    TargetCurrentTime = TargetDuration - KINDA_SMALL_NUMBER;
                }
            }
        }

        // 포즈 가져오기
        FPoseContext SourcePose(this);
        if (AnimSequence)
        {
            AnimSequence->GetAnimationPose(SourcePose, FAnimExtractContext(CurrentTime, false));
        }
        else
        {
            SourcePose.ResetToRefPose(this->GetRequiredBoneLocalTransforms());
        }

        FPoseContext TargetPose(this);
        if (TargetAnimSequence)
        {
            TargetAnimSequence->GetAnimationPose(TargetPose, FAnimExtractContext(TargetCurrentTime, false));
        }
        else
        {
            TargetPose.ResetToRefPose(this->GetRequiredBoneLocalTransforms());
        }

        // 포즈 블렌딩 (Output 포즈에 직접 결과 저장)
        Output.ResetToRefPose(this->GetRequiredBoneLocalTransforms());
        Output.AnimInstance = this;

        const int32 NumBones = Output.Pose.BoneTransforms.Num();
        if (NumBones == SourcePose.Pose.BoneTransforms.Num() && NumBones == TargetPose.Pose.BoneTransforms.Num())
        {
            for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
            {
                const FMatrix& SourceTarnsform = SourcePose.Pose.BoneTransforms[BoneIndex];
                const FMatrix& TargetTransform = TargetPose.Pose.BoneTransforms[BoneIndex];

                //// 각 요소 별로 블렌딩
                //FVector SourceTranslation = JungleMath::DecomposeTranslation(SourceTarnsform);
                //FQuat SourceRotation = JungleMath::DecomposeRotation(SourceTarnsform);
                //FVector SourceScale = JungleMath::DecomposeScale(SourceTarnsform);

                //FVector TargetTranslation = JungleMath::DecomposeTranslation(TargetTransform);
                //FQuat TargetRotation = JungleMath::DecomposeRotation(TargetTransform);
                //FVector TargetScale = JungleMath::DecomposeScale(TargetTransform);

                FVector SourceTranslation = SourceTarnsform.GetTranslationVector();
                FQuat SourceRotation = SourceTarnsform.ToQuat();
                FVector SourceScale = SourceTarnsform.GetScaleVector();

                FVector TargetTranslation = TargetTransform.GetTranslationVector();
                FQuat TargetRotation = TargetTransform.ToQuat();
                FVector TargetScale = TargetTransform.GetScaleVector();


                FVector BlendedTranslation = FMath::Lerp(SourceTranslation, TargetTranslation, BlendAlpha);
                FQuat BlendedRotation = FQuat::Slerp(SourceRotation, TargetRotation, BlendAlpha);
                FVector BlendedScale = FMath::Lerp(SourceScale, TargetScale, BlendAlpha);

                // 블렌딩된 트랜스폼을 Output 포즈에 저장
                Output.Pose.BoneTransforms[BoneIndex] = JungleMath::CreateModelMatrix(BlendedTranslation, BlendedRotation, BlendedScale);
            }
        }
        else if (TargetAnimSequence) // 소스만 유효하지 않을 때
        {
            Output = TargetPose;
        }
        else if (AnimSequence) // 타겟만 유효하지 않을 때
        {
            Output = SourcePose;
        }

        // 블렌딩 완료 처리
        if (BlendAlpha >= 1.f)
        {
            bIsBlending = false;
            BlendTimeElapsed = 0.f;

            // 현재 애니메이션을 타겟 애니메이션으로 완전히 전환
            AnimSequence = TargetAnimSequence;
            CurrentTime = TargetCurrentTime;
            PlayRate = TargetPlayRate;
            bIsLooping = bTargetLooping;

            TargetAnimSequence = nullptr; // 타겟 정보 클리어
            PrevFrameNotifies.Empty(); // 새 애니메이션 시작이므로 노티파이 기록 초기화 (확인필요)
        }
    }
    else // 블렌딩이 아닐 때 
    {
        if (AnimSequence)
        {
            // 재생 속도(PlayRate)를 곱해서야 제대로 속도 조절이 됩니다.
            CurrentTime += DeltaSeconds * PlayRate;

    if (bIsLooping)
    {
        // 시퀀스 길이를 넘어가면 맨 앞으로 되돌리기
        CurrentTime = FMath::Fmod(CurrentTime, AnimSequence->GetPlayLength());
        if (CurrentTime < 0.f) 
        {
            CurrentTime = AnimSequence->GetPlayLength() + CurrentTime;  // 음수로 나온 값 반영해주기
        }
    }
    else
    {
        // 한 번만 재생할 땐 끝 시간을 넘지 않도록 고정
        CurrentTime = FMath::Clamp(CurrentTime, 0.f, AnimSequence->GetPlayLength());
    }
    
    FPoseContext Pose(this);

            FAnimExtractContext Extract(CurrentTime, false);

            AnimSequence->GetAnimationPose(Pose, Extract);

            Output.Pose = Pose.Pose;
            // Output.Curve = Pose.Curve;
        }
    }
}

void UMyAnimInstance::SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InBlendDuration, float InPlayRate)
{
    if (NewSequence == nullptr)
    {
        return;
    }

    // 다른 애니메이션이 적용되는 경우 블렌딩 처리
    StartCrossfade(NewSequence, bLooping, InBlendDuration, InPlayRate);
}

void UMyAnimInstance::SetPlaying(bool bInPlaying)
{
    bIsPlaying = bInPlaying;
}

void UMyAnimInstance::SetLooping(bool bInLooping)
{
    bIsLooping = bInLooping;
}

FPoseContext& UMyAnimInstance::GetOutput()
{
    return Output;
}

void UMyAnimInstance::StartCrossfade(UAnimSequence* NewTargetSequence, bool InbTargetLooping, float InBlendDuration, float InTargetPlayRate)
{
    if (!AnimSequence || !NewTargetSequence || InBlendDuration <= 0.f)
    {
        return;
    }

    // 이미 NewTargetSequence로 블렌딩 중이라면 바로 변경
    if (bIsBlending && TargetAnimSequence != NewTargetSequence)
    {
        bIsBlending = false;
        return;
    }

    // 타겟 설정
    TargetAnimSequence = NewTargetSequence;
    TargetCurrentTime = 0.f;
    TargetPlayRate = InTargetPlayRate;
    bTargetLooping = InbTargetLooping;

    // 블렌딩 시작
    bIsBlending = true;
    BlendDurationTotal = InBlendDuration;
    BlendTimeElapsed = 0.f;

    bIsPlaying = true;
}
