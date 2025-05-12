#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimTypes.h"

class USkeletalMesh;

class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject)
     
public:
    UAnimInstance();
    ~UAnimInstance() = default;

    void TriggerAnimNotifies(float DeltaSeconds);
    const FBoneContainer& GetRequiredBones() const;

    void UpdateAnimation(float DeltaSeconds);

    virtual void NativeInitializeAnimation();
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    FBoneContainer RequiredBones; // 본 계층 정보

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);
    USkeletalMesh* GetSkeletalMesh() const;

protected:
    USkeletalMesh* SkeletalMesh; // 스켈레탈 메시
};

