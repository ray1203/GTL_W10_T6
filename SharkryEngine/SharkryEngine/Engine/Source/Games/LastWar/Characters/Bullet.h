#pragma once

#include "GameFramework/Actor.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class ABullet : public AActor
{
    DECLARE_CLASS(ABullet, AActor)

public:
    ABullet();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;

public:
    void InitBullet(float InBulletSpeed, float InBulletDamage);

private:
    void OnBeginOverlap(AActor* OtherActor);

public:
    virtual void RegisterLuaType(sol::state& Lua); // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties(); // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
    void SetBulletSpeed(float InBulletSpeed) { BulletSpeed = InBulletSpeed; }
    float GetBulletSpeed() const { return BulletSpeed; }
    void SetBulletDamage(float InBulletDamage) { BulletDamage = InBulletDamage; }
    float GetBulletDamage() const { return BulletDamage; }

private:
    float BulletSpeed = 0.0f;
    float BulletDamage = 0.0f;

protected:

    UPROPERTY
    (UStaticMeshComponent*, BodyMesh, = nullptr);

    UPROPERTY
    (UCapsuleComponent*, CollisionCapsule, = nullptr);
};

