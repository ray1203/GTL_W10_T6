#pragma once
#include "GameFramework/Character.h"

class AEnemyCharacter : public ACharacter
{
    DECLARE_CLASS(AEnemyCharacter, ACharacter)

public:
    AEnemyCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /**
        * 해당 액터의 직렬화 가능한 속성들을 문자열 맵으로 반환합니다.
        * 하위 클래스는 이 함수를 재정의하여 자신만의 속성을 추가해야 합니다.
        */
    void GetProperties(TMap<FString, FString>& OutProperties) const override;

    /** 저장된 Properties 맵에서 액터의 상태를 복원합니다. */
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
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
    (float, Speed, = 1.0f)

    UPROPERTY
    (float, Damage, = 50.0f)
};

