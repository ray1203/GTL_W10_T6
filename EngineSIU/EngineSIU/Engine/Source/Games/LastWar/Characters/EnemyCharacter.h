#pragma once
#include "GameFramework/Character.h"

class AEnemyCharacter : public ACharacter
{
    DECLARE_CLASS(AEnemyCharacter, ACharacter)

public:
    AEnemyCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY
    (float, Health, = 100.0f)

    UPROPERTY
    (float, Speed, = 1.0f)

    UPROPERTY
    (float, Damage, = 10.0f)
};

