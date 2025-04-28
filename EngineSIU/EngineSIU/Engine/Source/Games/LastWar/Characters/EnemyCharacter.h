#pragma once
#include "GameFramework/Character.h"

class AEnemyCharacter : public ACharacter
{
    DECLARE_CLASS(AEnemyCharacter, ACharacter)

public:
    AEnemyCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    // === Getter ===
    float GetHealth() const { return Health; }
    float GetSpeed() const { return Speed; }
    float GetDamage() const { return Damage; }

    // === Setter ===
    void SetHealth(float NewHealth) { Health = NewHealth; }
    void SetSpeed(float NewSpeed) { Speed = NewSpeed; }
    void SetDamage(float NewDamage) { Damage = NewDamage; }

private:
    UPROPERTY
    (float, Health, = 100.0f)

    UPROPERTY
    (float, Speed, = 5.0f)

    UPROPERTY
    (float, Damage, = 10.0f)
};

