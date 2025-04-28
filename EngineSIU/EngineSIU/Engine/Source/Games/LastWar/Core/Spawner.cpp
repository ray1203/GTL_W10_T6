// SpawnerActor.cpp
#include "Spawner.h"
#include "World/World.h"
#include "UObject/Class.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

ASpawnerActor::ASpawnerActor()
{
    USceneComponent* newComponent = AddComponent<USceneComponent>("SpawnerLocation");
    RootComponent = newComponent;
}

void ASpawnerActor::BeginPlay()
{
    Super::BeginPlay();
}

void ASpawnerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


UObject* ASpawnerActor::Duplicate(UObject* InOuter)
{
    UObject* NewObj = Super::Duplicate(InOuter); // AActor::Duplicate() 호출
    ASpawnerActor* NewSpawner = Cast<ASpawnerActor>(NewObj);
    return NewSpawner;
}

AActor* ASpawnerActor::SpawnActorLua(const std::string& ClassName, const FVector& Location)

{
    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    UClass* SpawnClass = UClass::FindClass(FString(ClassName.c_str()));

    if (!SpawnClass)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: Cannot find class '%s'"), ClassName);
        return nullptr;
    }

    // 여기서 바로 SpawnActor 호출
    AActor* NewActor = World->SpawnActor(SpawnClass);

    if (!NewActor)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: SpawnActor returned null for '%s'"), ClassName);
        return nullptr;
    }

    // 위치만 세팅
    NewActor->SetActorLocation(Location);

    return NewActor;
}

void ASpawnerActor::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ASpawnerActor, AActor,
        "SpawnActorLua", &ThisClass::SpawnActorLua,
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation) 
    )
}


bool ASpawnerActor::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();

    if (!LuaScriptComponent)
    {
        return false;
    }

    if (LuaScriptComponent->GetLuaSelfTable().valid())
        LuaScriptComponent->GetLuaSelfTable()["this"] = this;

    return true;
}

