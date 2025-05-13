#include "AnimationStateMachine.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimInstances/AnimSingleNodeInstance.h"

UAnimationStateMachine::UAnimationStateMachine()
{
}

void UAnimationStateMachine::ProcessState()
{
    if (!Pawn) return;

    PreviousState = CurrentState;

    if (Pawn->PendingMovement.IsNearlyZero())
    {
        CurrentState = AS_Idle;
    }
    else if (Pawn->GetVelocity() <= Pawn->GetWalkSpeed())
    {
        CurrentState = AS_Walk;
    }
    else if (Pawn->GetVelocity() <= Pawn->GetRunSpeed())
    {
        CurrentState = AS_Run;
    }
    else
    {
        CurrentState = AS_Jump;
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

    UAnimSingleNodeInstance* AnimInstance = MeshComp->GetSingleNodeInstance();
    if (!AnimInstance) return;

    // 애니메이션 시퀀스 변경
    switch (CurrentState)
    {
    case AS_Idle:
        MeshComp->PlayAnimation(AnimInstance->GetIdleAnimSequence(), true);
        break;
    case AS_Walk:
        MeshComp->PlayAnimation(AnimInstance->GetWalkAnimSequence(), true);
        break;
    case AS_Run:
        MeshComp->PlayAnimation(AnimInstance->GetRunAnimSequence(), true);
        break;
    case AS_Jump:
        MeshComp->PlayAnimation(AnimInstance->GetJumpAnimSequence(), false);
        break;
    }
}
