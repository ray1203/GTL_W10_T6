#pragma once

#include "Engine/Animation/AnimInstance.h"
#include "Engine/Source/Runtime/Engine/Animation/AnimSequence.h"

class UAnimSingleNodeInstance : public UAnimInstance 
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UObject)

public:
    UAnimSingleNodeInstance();
    ~UAnimSingleNodeInstance() = default;

    void SetAnimSequence(UAnimSequence* InAnimSequence) { AnimSequence = InAnimSequence; }

    FPoseContext GetOutput() { return Output; }

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UAnimSequence* AnimSequence;
    FPoseContext Output;

    float CurrentTime = 0.0f;
    
    bool bPlaying = true;
    float PlayRate = 1.0f;
    bool bLooping = true;
};
