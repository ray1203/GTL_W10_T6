#pragma once
#include "AnimationAsset.h"
#include "AnimTypes.h"

class UAnimDataModel;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)

public:
    UAnimSequenceBase();
    ~UAnimSequenceBase() = default;

public:
    UAnimDataModel* GetDataModel() const;
    virtual void GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext) = 0;

    /** Animation notifies, sorted by time (earliest notification first). */
    UPROPERTY(TArray<struct FAnimNotifyEvent>, Notifies);

protected:
    UAnimDataModel* AnimDataModel;

};

