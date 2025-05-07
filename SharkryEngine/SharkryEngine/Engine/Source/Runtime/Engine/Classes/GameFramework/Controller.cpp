#include "Controller.h"
#include "Pawn.h"
#include "Components/InputComponent.h"

void AController::BeginPlay()
{
    Super::BeginPlay();
}

void AController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AController::Destroyed()
{
    Super::Destroyed();
}

void AController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AController::Possess(APawn* InPawn)
{
    if (Pawn == InPawn)
    {
        return;
    }

    // 기존에 소유하던 Pawn이 있으면 해제
    UnPossess();

    if (InPawn)
    {
        // 새 Pawn을 소유 상태로 설정
        Pawn = InPawn;

        // Pawn 쪽 최초 PossessedBy 처리: EnableInput 등
        Pawn->PossessedBy(this);
    }
}

void AController::UnPossess()
{
    if (Pawn)
    {
        // Pawn 쪽 UnPossessed 처리: DisableInput 등
        Pawn->UnPossessed();
        Pawn = nullptr;
    }
}

APawn* AController::GetPawn() const
{
    return Pawn;
}

void AController::SetPawn(APawn* InPawn)
{
    Pawn = InPawn;
}

void AController::SetupInputComponent()
{
    // 입력 처리할 컴포넌트가 없으면 새로 생성
    if (!InputComponent)
    {
        InputComponent = FObjectFactory::ConstructObject<UInputComponent>(this);
    }
}

FRotator AController::GetControlRotation() const
{
    return FRotator();
}

void AController::SetControlRotation(const FRotator& NewRotation)
{
}
