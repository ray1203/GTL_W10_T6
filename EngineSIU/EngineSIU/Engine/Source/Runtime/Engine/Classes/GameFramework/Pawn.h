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
    // TODO Check Duplicate Controller

    /**
* 해당 액터의 직렬화 가능한 속성들을 문자열 맵으로 반환합니다.
* 하위 클래스는 이 함수를 재정의하여 자신만의 속성을 추가해야 합니다.
*/
    void GetProperties(TMap<FString, FString>& OutProperties) const override;

    /** 저장된 Properties 맵에서 액터의 상태를 복원합니다. */
    void SetProperties(const TMap<FString, FString>& InProperties) override;

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


    AController* GetController() const { return Controller; }

protected:
    UPROPERTY
    (AController*, Controller, = nullptr) // 현재 조종 중인 컨트롤러

protected:
    FVector PendingMovement;

    UPROPERTY
    (float, MoveSpeed, = 6.0f)
};

