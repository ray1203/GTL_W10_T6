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

    TMap<FName, uint32> GetRequiredBoneNames();

    TArray<FMatrix> GetRequiredBoneLocalTransforms();

    void UpdateAnimation(float DeltaSeconds);

    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }


private:
    USkeletalMesh* SkeletalMesh = nullptr;
};

