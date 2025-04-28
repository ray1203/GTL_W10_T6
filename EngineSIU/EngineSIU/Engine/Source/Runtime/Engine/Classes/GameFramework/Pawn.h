#pragma once
#include "Actor.h"

class AController;
class APlayerController;
class UInputComponent;

class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)

public:
    APawn() = default;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // TODO Check Duplicate

public:
    /** Pawn을 Controller에 의해 점유(Possess)될 때 호출 */
    virtual void PossessedBy(AController* NewController);

    /** Pawn이 Controller에서 해제(UnPossess)될 때 호출 */
    virtual void UnPossessed();

    /** 플레이어 입력을 수신하고 처리 */
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

    /** 입력 처리용 함수 */
    virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f);

    /** 입력을 받아 회전 처리 */
    virtual void AddControllerYawInput(float Value);
    virtual void AddControllerPitchInput(float Value);

    /** 시야 관련 함수 */
    virtual FVector GetPawnViewLocation() const;
    virtual FRotator GetViewRotation() const;

    virtual void EnableInput(APlayerController* PlayerController) override;
    virtual void DisableInput(APlayerController* PlayerController) override;

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.


    UPROPERTY
    (AController*, Controller, = nullptr) // 현재 조종 중인 컨트롤러

protected:
    FVector PendingMovement;

    UPROPERTY
    (float, MoveSpeed, = 5.0f)
};

