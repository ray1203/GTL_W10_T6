#include "AnimationStateMachine.h"
#include "GameFramework/Pawn.h"

UAnimationStateMachine::UAnimationStateMachine()
{
}

void UAnimationStateMachine::ProcessState()
{
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
