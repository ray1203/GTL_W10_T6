#pragma once
#include "AnimationAsset.h"

class UAnimDataModel;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)

public:
    UAnimSequenceBase();
    ~UAnimSequenceBase() = default;

public:
    virtual UAnimDataModel* GetDataModel() const override;

    /** Animation notifies, sorted by time (earliest notification first). */
    //UPROPERTY(TArray<struct FAnimNotifyEvent>, Notifies);



};

