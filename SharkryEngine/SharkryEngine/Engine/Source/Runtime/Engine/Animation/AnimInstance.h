#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimTypes.h"
#include "Components/Mesh/SkeletalMesh.h"

class USkeletalMeshComponent;
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
    virtual void UpdateNotify(float DeltaSeconds);

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }
    void SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComp);

    virtual FPoseContext& GetOutput();

    virtual void SetAnimationSequence(UAnimSequence* NewSequence, bool bLooping, float InBlendDuration = 1.f, float InPlayRate = 1.f);
    virtual void SetPlaying(bool bInPlaying);
    virtual void SetLooping(bool bInLooping);

    virtual void StartCrossfade(UAnimSequence* NewTargetSequence, bool bTargetLooping, float InBlendDuration, float InTargetPlayRate = 1.f);

protected:
    USkeletalMesh* SkeletalMesh = nullptr;
    USkeletalMeshComponent* SkeletalMeshComp = nullptr;
    FPoseContext Output;
};

