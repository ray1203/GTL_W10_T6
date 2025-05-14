#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Components/Mesh/SkeletalMesh.h"

class UAnimSequence;
class USkeletalMeshComponent;

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
    virtual void UpdateNotify(float DeltaSeconds);

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }
    void SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComp);

    void SetIdleAnimSequence(UAnimSequence* InIdleAnimSequence) { IdleAnimSequence = InIdleAnimSequence; }
    void SetWalkAnimSequence(UAnimSequence* InWalkAnimSequence) { WalkAnimSequence = InWalkAnimSequence; }
    void SetRunAnimSequence(UAnimSequence* InRunAnimSequence) { RunAnimSequence = InRunAnimSequence; }
    void SetJumpAnimSequence(UAnimSequence* InJumpAnimSequence) { JumpAnimSequence = InJumpAnimSequence; }
    UAnimSequence* GetIdleAnimSequence() { return IdleAnimSequence; }
    UAnimSequence* GetWalkAnimSequence() { return WalkAnimSequence; }
    UAnimSequence* GetRunAnimSequence() { return RunAnimSequence; }
    UAnimSequence* GetJumpAnimSequence() { return JumpAnimSequence; }

protected:
    USkeletalMesh* SkeletalMesh = nullptr;
    USkeletalMeshComponent* SkeletalMeshComp = nullptr;

    UAnimSequence* IdleAnimSequence = nullptr;
    UAnimSequence* WalkAnimSequence = nullptr;
    UAnimSequence* RunAnimSequence = nullptr;
    UAnimSequence* JumpAnimSequence = nullptr;
};

