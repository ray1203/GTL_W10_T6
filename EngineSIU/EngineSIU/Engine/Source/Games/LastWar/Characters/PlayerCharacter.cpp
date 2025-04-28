#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Delegates/DelegateCombination.h"
#include "EnemyCharacter.h"
#include "Components/LuaScriptComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "GameFramework/PlayerController.h"

APlayerCharacter::APlayerCharacter()
{
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));

    FollowCamera = AddComponent<UCameraComponent>("PlayerCamera");
    FollowCamera->SetupAttachment(RootComponent);
}

UObject* APlayerCharacter::Duplicate(UObject* InOuter)
{
    UObject* NewActor = Super::Duplicate(InOuter);
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(NewActor);
    PlayerCharacter->FollowCamera = GetComponentByFName<UCameraComponent>("PlayerCamera");

    return NewActor;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    OnActorBeginOverlapHandle = OnActorBeginOverlap.AddDynamic(this, &APlayerCharacter::HandleOverlap);
}
  
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Forward = GetActorForwardVector();
    FVector BackOffset = -Forward * 3.0f;
    FVector UpOffset = FVector(0.0f, 0.0f, 3.0f);
    FVector CamLocation = GetActorLocation() + BackOffset + UpOffset;
    FollowCamera->SetLocation(CamLocation);
    FollowCamera->SetRotation(GetActorRotation());

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Bind input actions and axes here  
    PlayerInputComponent->BindAxis("MoveForward", [this](float Value) { MoveForward(Value); });
    PlayerInputComponent->BindAxis("MoveRight", [this](float Value) { MoveRight(Value); });
}

void APlayerCharacter::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Health"), std::to_string(Health));
    OutProperties.Add(TEXT("Speed"), std::to_string(Speed));
    OutProperties.Add(TEXT("AttackDamage"), std::to_string(AttackDamage));
}

void APlayerCharacter::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Health"));
    if (TempStr)
    {
        Health = std::stof(GetData(*TempStr));
    }
    TempStr = InProperties.Find(TEXT("Speed"));
    if (TempStr)
    {
        Speed = std::stof(GetData(*TempStr));
    }
    TempStr = InProperties.Find(TEXT("AttackDamage"));
    if (TempStr)
    {
        AttackDamage = std::stof(GetData(*TempStr));
    }
}

void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void APlayerCharacter::HandleOverlap(AActor* OtherActor)  
{  
    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor);
    if (Enemy)  
    {  
        if (IsActorBeingDestroyed())  
        {  
            return;  
        }  
        UE_LOG(LogLevel::Display, "Handle Overlap %s,  %s", GetData(OtherActor->GetName()), GetData(GetName()));  

        if (LuaScriptComponent)
        {
            LuaScriptComponent->ActivateFunction("OnOverlap", Enemy, Enemy->GetDamage());
        }
    }

    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
        DisableInput(Cast<APlayerController>(Controller));
    }
}

void APlayerCharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayerCharacter, ACharacter,
        "Health", sol::property(&ThisClass::GetHealth, &ThisClass::SetHealth),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "AttackDamage", sol::property(&ThisClass::GetAttackDamage, &ThisClass::SetAttackDamage)
    )
}

bool APlayerCharacter::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();
    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }
    LuaTable["this"] = this;

    return true;
}
 
