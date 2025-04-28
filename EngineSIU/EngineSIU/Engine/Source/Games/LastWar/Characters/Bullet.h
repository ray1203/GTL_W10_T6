#pragma once

#include "GameFramework/Actor.h"

class ABullet : public AActor
{
    DECLARE_CLASS(ABullet, AActor)

public:
    ABullet() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;

    void OnBeginOverlap(AActor* OtherActor);

public:
    virtual void RegisterLuaType(sol::state& Lua); // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties(); // LuaEnv에서 사용할 멤버 변수 등록 함수.

};

