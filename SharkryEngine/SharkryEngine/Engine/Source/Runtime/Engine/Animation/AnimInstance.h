#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/SkeletalMesh.h"

class UAnimSequence;
class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject)

public:
    UAnimInstance();
    ~UAnimInstance() = default;

    virtual void NativeInitializeAnimation();
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void TriggerAnimNotifies(float DeltaSeconds);

    TMap<FName, uint32> GetRequiredBoneNames();

    TArray<FMatrix> GetRequiredBoneLocalTransforms();

    void UpdateAnimation(float DeltaSeconds);

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }

    UAnimSequence* IdleAnimSequence = nullptr;
    UAnimSequence* WalkAnimSequence = nullptr;
    UAnimSequence* RunAnimSequence = nullptr;
    UAnimSequence* JumpAnimSequence = nullptr;
private:
    USkeletalMesh* SkeletalMesh = nullptr;
};

