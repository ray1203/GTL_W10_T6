#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject)
     
public:
    UAnimInstance();
    ~UAnimInstance() = default;

    void TriggerAnimNotifies(float DeltaSeconds);

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds);

};

