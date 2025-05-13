#pragma once
#include "Pawn.h"

class UInputComponent;
class AController;
class UMeshComponent;
class UCapsuleComponent;
struct FAnimNotifyEvent;

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)

public:
    ACharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    UObject* Duplicate(UObject* InOuter) override;

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    void HandleAnimNotify(const FAnimNotifyEvent& Notify);

protected:
    UPROPERTY
    (UMeshComponent*, BodyMesh, = nullptr);

    UPROPERTY
    (UCapsuleComponent*, CollisionCapsule, = nullptr);
};

