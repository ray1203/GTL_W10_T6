#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject)

public:
    UAnimInstance();

    void TriggerAnimNotifies(float DeltaSeconds);
};
