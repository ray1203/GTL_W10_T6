#pragma once
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

class USkeletalMeshComponent;


class ASkeletalMeshActor : public ACharacter
{
    DECLARE_CLASS(ASkeletalMeshActor, ACharacter)

public:
    ASkeletalMeshActor();

    UObject* Duplicate(UObject* InOuter) override;

    virtual void BeginPlay() override;

    USkeletalMeshComponent* GetSkeletalMeshComponent() const;

    void SetSkeletalMesh(const FWString& SkelName);

    void SetAnimationAsset();

protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr);
};
