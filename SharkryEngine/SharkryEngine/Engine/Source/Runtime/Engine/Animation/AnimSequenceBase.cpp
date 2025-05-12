#include "AnimSequenceBase.h"
#include "AnimData/AnimDataModel.h"
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
