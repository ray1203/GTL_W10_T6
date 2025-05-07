#pragma once
#include "Actor.h"

class APawn;

class AController : public AActor
{
    DECLARE_CLASS(AController, AActor)

public:
    AController() = default;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // TODO Check Duplicate

public:
    // === Pawn 소유 관리 ===
    virtual void Possess(APawn* InPawn);
    virtual void UnPossess();
    APawn* GetPawn() const;
    void SetPawn(APawn* InPawn);

    // === 입력 처리 ===
    virtual void SetupInputComponent(); // PlayerController가 override
    UInputComponent* InputComponent;

    // === 시야/컨트롤 회전 ===
    virtual FRotator GetControlRotation() const;
    virtual void SetControlRotation(const FRotator& NewRotation);

protected:
    UPROPERTY
    (APawn*, Pawn, = nullptr)
    
};

