#pragma once

#include "GameFramework/Actor.h"

class UHeightFogComponent;

class AHeightFogActor : public AActor
{
    DECLARE_CLASS(AHeightFogActor, AActor)

public:
    AHeightFogActor();
    UObject* Duplicate(UObject* InOuter) override;

    UHeightFogComponent* HeightFogComponent;
};
