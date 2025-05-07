#include "HeightFogActor.h"

#include "Components/HeightFogComponent.h"

AHeightFogActor::AHeightFogActor()
{
    HeightFogComponent = AddComponent<UHeightFogComponent>("UHeightFogComponent_0");
}

UObject* AHeightFogActor::Duplicate(UObject* InOuter)
{
    UObject* NewActor = AActor::Duplicate(InOuter);
    AHeightFogActor* HeightFogActor = Cast<AHeightFogActor>(NewActor);
    HeightFogActor->HeightFogComponent = GetComponentByFName<UHeightFogComponent>("UHeightFogComponent_0");

    return NewActor;
}
