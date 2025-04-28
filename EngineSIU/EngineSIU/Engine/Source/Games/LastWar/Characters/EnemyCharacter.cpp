#include "EnemyCharacter.h"
#include "Components/InputComponent.h"  
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"

AEnemyCharacter::AEnemyCharacter()
{
    BodyMesh = AddComponent<UStaticMeshComponent>("Enemy");
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    BodyMesh->SetupAttachment(RootComponent);

    // Initialize properties or components here
    Health = 100.0f;
    Speed = 1.0f;
    Damage = 10.0f;
    SetActorLocation(FVector(20.0f, 0.0f, 0.0f));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

