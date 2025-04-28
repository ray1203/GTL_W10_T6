#pragma once
#include "GameFramework/Character.h"

class UCameraComponent;
class UInputComponent;
class APlayerCharacter : public ACharacter
{
    DECLARE_CLASS(APlayerCharacter, ACharacter)

public:
    APlayerCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


    // === 이동 관련 ===
    void MoveForward(float Value);
    void MoveRight(float Value);

    void HandleOverlap(AActor* OtherActor);

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    // === Getter ===
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    float GetHealth() const { return Health; }
    float GetSpeed() const { return Speed; }
    float GetAttackDamage() const { return AttackDamage; }

    // === Setter ===
    void SetHealth(float NewHealth) { Health = NewHealth; }
    void SetSpeed(float NewSpeed) { Speed = NewSpeed; }
    void SetAttackDamage(float NewAttackDamage) { AttackDamage = NewAttackDamage; }

private:
    UPROPERTY
    (UCameraComponent*, FollowCamera, = nullptr);

    UPROPERTY
    (float, Health, = 100.0f)

    UPROPERTY
    (float, Speed, = 1.0f)

    UPROPERTY
    (float, AttackDamage, = 10.0f)
};

