#include "AnimSequenceBase.h"
#include "AnimData/AnimDataModel.h"
#include "Engine/Source/Runtime/Engine/Animation/AnimNotify.h"

UAnimSequenceBase::UAnimSequenceBase()
{
}

UAnimDataModel* UAnimSequenceBase::GetDataModel() const
{
    return AnimDataModel;
}

void UAnimSequenceBase::GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext)
{
    // 자식인 UAnimSequence에서 구현
}

void UAnimSequenceBase::GetAnimNotifies(const float& StartTime, const float& DeltaTime, const bool bAllowLooping, TArray<FAnimNotifyEvent*>& OutNotifies)
{
    // 자식인 UAnimSequence에서 구현
}
