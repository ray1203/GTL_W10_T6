#pragma once
#include "AnimationAsset.h"
#include "AnimData/AnimDataModel.h"

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)

public:
    UAnimSequenceBase();
    ~UAnimSequenceBase() = default;

public:
    UAnimDataModel* GetDataModel() const;

    /** Animation notifies, sorted by time (earliest notification first). */
    UPROPERTY(TArray<FAnimNotifyEvent>, Notifies);



};

