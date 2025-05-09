#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UAnimDataModel;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UObject
{
    DECLARE_CLASS(UAnimSequenceBase, UObject)

public:
    UAnimSequenceBase();
    ~UAnimSequenceBase() = default;

public:
    UAnimDataModel* GetDataModel() const;

    /** Animation notifies, sorted by time (earliest notification first). */
    UPROPERTY(TArray<struct FAnimNotifyEvent>, Notifies);



};

