#include "Level.h"
#include "GameFramework/Actor.h"
#include "UObject/Casts.h"
#include "World/World.h"


void ULevel::InitLevel(UWorld* InOwningWorld)
{
    OwningWorld = InOwningWorld;

}

void ULevel::Release()
{
    const auto CopiedActors = Actors;
    for (AActor* Actor : CopiedActors)
    {
        Actor->EndPlay(EEndPlayReason::WorldTransition);
        Actor->Destroy();
    }
    Actors.Empty();
}

UObject* ULevel::Duplicate(UObject* InOuter)
{
    ThisClass* NewLevel = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewLevel->OwningWorld = OwningWorld;
    NewLevel->LevelName = LevelName;

    for (AActor* Actor : Actors)
    {
        NewLevel->Actors.Emplace(static_cast<AActor*>(Actor->Duplicate(InOuter)));
    }

    return NewLevel;
}
