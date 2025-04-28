#include "World.h"

#include "Actors/Cube.h"
#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "UnrealEd/SceneManager.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UWorld* UWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel = FObjectFactory::ConstructObject<ULevel>(this);
    ActiveLevel->InitLevel(this);
}

UObject* UWorld::Duplicate(UObject* InOuter)
{
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate(InOuter));
    NewWorld->ActiveLevel = Cast<ULevel>(ActiveLevel->Duplicate(NewWorld));
    NewWorld->ActiveLevel->InitLevel(NewWorld);
    return NewWorld;
}

void UWorld::BeginPlay()
{
    for (AActor* Actor : ActiveLevel->Actors)
        {
            if (Actor->GetWorld() == this)
            {
                Actor->BeginPlay();
                if (PendingBeginPlayActors.Contains(Actor))
                {
                    PendingBeginPlayActors.Remove(Actor);
                }
            }
        }
}

void UWorld::Tick(float DeltaTime)
{
    if (WorldType != EWorldType::Editor)
    {
        for (AActor* Actor : PendingBeginPlayActors)
        {
            Actor->BeginPlay();
        }
        PendingBeginPlayActors.Empty();
    }
    TArray<AActor*> ActorsCopy = GetActiveLevel()->Actors;

    for (AActor* Actor : ActorsCopy)
    {
        if (!Actor || Actor->IsActorBeingDestroyed())
            continue;

        Actor->UpdateOverlaps();
    }

    for (AActor* Actor : ActorsCopy)
    {
        if (!Actor || Actor->IsActorBeingDestroyed())
            continue;

        Actor->ProcessOverlaps();
    }

    if (!PendingDestroyActors.IsEmpty())
    {
        for (AActor* Actor : PendingDestroyActors)
        {
            ActiveLevel->Actors.Remove(Actor);
            GUObjectArray.MarkRemoveObject(Actor);
        }
        PendingDestroyActors.Empty();
    }
}

void UWorld::Release()
{
    if (ActiveLevel)
    {
        ActiveLevel->Release();
        GUObjectArray.MarkRemoveObject(ActiveLevel);
        ActiveLevel = nullptr;
    }

    GUObjectArray.ProcessPendingDestroyObjects();
}

AActor* UWorld::SpawnActor(UClass* InClass, FName InActorName)
{
    if (!InClass)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActor failed: ActorClass is null."));
        return nullptr;
    }

    if (InClass->IsChildOf<AActor>())
    {
        AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(InClass, this, InActorName));
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);

        NewActor->PostSpawnInitialize();
        return NewActor;
    }

    UE_LOG(LogLevel::Error, TEXT("SpawnActor failed: Class '%s' is not derived from AActor."), *InClass->GetName());
    return nullptr;
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);

    if (EditorEngine->GetSelectedActor() == ThisActor)
    {
        EditorEngine->DeselectActor(ThisActor);
    }
    if (EditorEngine->GetSelectedComponent() && ThisActor->GetComponentByFName<UActorComponent>(EditorEngine->GetSelectedComponent()->GetFName()))
    {
        EditorEngine->DeselectComponent(EditorEngine->GetSelectedComponent());
    }

    ThisActor->Destroyed();
    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    // 실제 Remove는 나중에
    PendingDestroyActors.Add(ThisActor);
    return true;
}

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}


