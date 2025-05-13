#include "AnimSingleNodeInstance.h"
#include "Animation/AnimationStateMachine.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "GameFramework/PlayerController.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{

}

void UAnimSingleNodeInstance::NativeInitializeAnimation()
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
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!StateMachine) return;

    StateMachine->ProcessState();

    CurrentState = StateMachine->CurrentState;
    PreviousState = StateMachine->PreviousState;

    if (CurrentState != PreviousState)
    {
        PreviousState = CurrentState;
        switch (CurrentState)
        {
        case AS_Idle:
            AnimSequence = IdleAnimSequence;
            break;
        case AS_Walk:
            AnimSequence = WalkAnimSequence;
            break;
        case AS_Run:
            AnimSequence = RunAnimSequence;
            break;
        case AS_Jump:
            AnimSequence = JumpAnimSequence;
            break;
        }
    }

    if (bPlaying && AnimSequence)
    {
        // 재생 속도(PlayRate)를 곱해서야 제대로 속도 조절이 됩니다.
        CurrentTime += DeltaSeconds * PlayRate;

        if (bLooping)
        {
            // 시퀀스 길이를 넘어가면 맨 앞으로 되돌리기
            CurrentTime = FMath::Fmod(CurrentTime, AnimSequence->GetPlayLength());
        }
        else
        {
            // 한 번만 재생할 땐 끝 시간을 넘지 않도록 고정
            CurrentTime = FMath::Clamp(CurrentTime, 0.f, AnimSequence->GetPlayLength());
        }
    }

    FPoseContext Pose(this);

    FAnimExtractContext Extract(CurrentTime, false);

    AnimSequence->GetAnimationPose(Pose, Extract);

    Output.Pose = Pose.Pose;
    // Output.Curve = Pose.Curve;
    

}
