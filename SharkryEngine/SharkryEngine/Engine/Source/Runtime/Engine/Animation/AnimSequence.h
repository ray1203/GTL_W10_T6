#pragma once
#include "AnimSequenceBase.h"

class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)

public:
    UAnimSequence();
    ~UAnimSequence() = default;

    virtual void GetAnimationPose(FPoseContext& PoseContext, const FAnimExtractContext& ExtractContext) const override;
};

