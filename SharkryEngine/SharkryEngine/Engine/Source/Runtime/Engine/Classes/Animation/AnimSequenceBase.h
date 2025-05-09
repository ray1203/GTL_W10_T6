#pragma once

#include "AnimInstance.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Public/Animation/AnimTypes.h"

class UAnimDataModel;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UAnimInstance
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimInstance)

public:
    UAnimSequenceBase();

    UAnimDataModel* GetDataMode() const;

private:
    TArray<FAnimNotifyEvent> Notifies;
};
