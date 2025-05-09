#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum EAnimState 
{
    AS_Idle,
    AS_Work,
    AS_Run,
    AS_Fly,
};

class UAnimationStateMachine : public UObject
{
    DECLARE_CLASS(UAnimationStateMachine, UObject)

public:
    UAnimationStateMachine();
    ~UAnimationStateMachine() = default;
    void ProcessState();
};

