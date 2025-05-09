#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/SkeletalMesh.h"

class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject)

public:
    UAnimInstance();
    ~UAnimInstance() = default;

    void TriggerAnimNotifies(float DeltaSeconds);

    TMap<FName, uint32> GetRequiredBones();

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds);


private:
    USkeletalMesh* SkeletalMesh = nullptr;
};

