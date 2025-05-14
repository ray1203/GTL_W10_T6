#pragma once
#include "AnimSequenceBase.h"

class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)

public:
    UAnimSequence();
    ~UAnimSequence() = default;

    float GetPlayLength();

    virtual void GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext) override;

    virtual void GetAnimNotifies(const float& StartTime, const float& DeltaTime,
        const bool bAllowLooping, TArray<FAnimNotifyEvent*>& OutNotifies) override;   
};

