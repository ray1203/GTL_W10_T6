#include "AnimSequenceBase.h"

UAnimSequenceBase::UAnimSequenceBase()
{
}

UAnimDataModel* UAnimSequenceBase::GetDataModel() const
{
    return AnimDataModel;
}

void UAnimSequenceBase::GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext)
{
}
