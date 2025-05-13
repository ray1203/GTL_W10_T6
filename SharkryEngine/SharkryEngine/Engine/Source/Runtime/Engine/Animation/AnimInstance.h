#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/SkeletalMesh.h"
#include "Engine/Animation/AnimTypes.h"

class USkeletalMeshComponent;

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

    virtual void UpdateNotify(float DeltaSeconds);

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }

    void SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComp);

    FPoseContext GetOutput() const;
     
protected:
    USkeletalMesh* SkeletalMesh = nullptr;
    USkeletalMeshComponent* SkeletalMeshComp = nullptr;
    FPoseContext Output;
};

