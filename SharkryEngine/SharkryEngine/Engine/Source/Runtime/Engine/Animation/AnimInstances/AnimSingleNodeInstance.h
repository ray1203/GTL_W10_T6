#pragma once

#include "Engine/Animation/AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/AnimSequence.h"

class UAnimSingleNodeInstance : public UAnimInstance 
{

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UAnimSequence* AnimSequence;
    FPoseContext Output;
};
