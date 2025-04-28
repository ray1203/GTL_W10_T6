// SpawnerActor.h
#pragma once
#include "GameFramework/Actor.h"

class ASpawnerActor : public AActor
{
    DECLARE_CLASS(ASpawnerActor, AActor)
public:
    ASpawnerActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
public:
    virtual UObject* Duplicate(UObject* InOuter) override;

    // Lua에서 호출할 함수들
    AActor* SpawnActorLua(const std::string& ClassName, const FVector& Location);
    virtual void RegisterLuaType(sol::state& Lua) override;
virtual bool BindSelfLuaProperties() override;

};
