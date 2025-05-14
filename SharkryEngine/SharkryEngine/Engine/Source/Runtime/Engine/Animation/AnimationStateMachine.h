#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum EAnimState 
{
    AS_Idle,
    AS_Walk,
    AS_Run,
    AS_Jump,
};

class APawn;
class UAnimationStateMachine : public UObject
{
    DECLARE_CLASS(UAnimationStateMachine, UObject)

public:
    UAnimationStateMachine();
    ~UAnimationStateMachine() = default;
    void ProcessState();
    void Update(float DeltaSeconds);
    void ApplyStateChange();

    void SetPawn(APawn* InPawn) { Pawn = InPawn; }

    EAnimState CurrentState;
    EAnimState PreviousState;

private:
    APawn* Pawn = nullptr;
};

