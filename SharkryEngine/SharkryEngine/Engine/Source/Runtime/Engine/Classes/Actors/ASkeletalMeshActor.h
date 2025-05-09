#pragma once
#include "GameFramework/Actor.h"

class USkeletalMeshComponent;


class ASkeletalMeshActor : public AActor
{
    DECLARE_CLASS(ASkeletalMeshActor, AActor)

public:
    ASkeletalMeshActor();

    UObject* Duplicate(UObject* InOuter) override;

    USkeletalMeshComponent* GetSkeletalMeshComponent() const;

protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr);
};
