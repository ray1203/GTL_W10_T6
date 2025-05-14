#include "AnimationStateMachine.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimInstances/MyAnimInstance.h"

UAnimationStateMachine::UAnimationStateMachine()
{
}

void UAnimationStateMachine::ProcessState()
{
    if (!Pawn) return;

    PreviousState = CurrentState;

    if (Pawn->bIsJumping)
    {
        CurrentState = AS_Jump;
    }
    else if (Pawn->PendingMovement.IsNearlyZero())
    {
        CurrentState = AS_Idle;
    }
    else if (Pawn->GetVelocity() <= Pawn->GetRunSpeed())
    {
        CurrentState = AS_Walk;
    }
    else if (Pawn->GetVelocity() > Pawn->GetRunSpeed())
    {
        CurrentState = AS_Run;
    }
    else
    {
        CurrentState = AS_Idle;
    }
}

void UAnimationStateMachine::Update(float DeltaSeconds)
{
    ProcessState();

    // 상태가 바뀌었을 때만 애니메이션 재생 호출
    if (CurrentState != PreviousState)
    {
        ApplyStateChange();
    }
}

void UAnimationStateMachine::ApplyStateChange()
{
    if (!Pawn) return;

    USkeletalMeshComponent* MeshComp = Pawn->GetComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp) return;

    UMyAnimInstance* AnimInstance = Cast<UMyAnimInstance>(MeshComp->GetAnimInstance());
    if (!AnimInstance) return;

    // 애니메이션 시퀀스 변경
    switch (CurrentState)
    {
    case AS_Idle:
        MeshComp->PlayAnimation(EAnimationMode::AnimationCustomMode, AnimInstance->GetIdleAnimSequence(), true);
        break;
    case AS_Walk:
        MeshComp->PlayAnimation(EAnimationMode::AnimationCustomMode, AnimInstance->GetWalkAnimSequence(), true);
        break;
    case AS_Run:
        MeshComp->PlayAnimation(EAnimationMode::AnimationCustomMode, AnimInstance->GetRunAnimSequence(), true);
        break;
    case AS_Jump:
        MeshComp->PlayAnimation(EAnimationMode::AnimationCustomMode, AnimInstance->GetJumpAnimSequence(), false);
        break;
    }
}
