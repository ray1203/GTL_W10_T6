#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Delegates/DelegateCombination.h"
#include "EnemyCharacter.h"
#include "Wall.h"
#include "World/World.h"
#include "Components/LuaScriptComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/BoxComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "GameFramework/PlayerController.h"
#include "Games/LastWar/UI/LastWarUI.h"
#include "Audio/AudioManager.h"

APlayerCharacter::APlayerCharacter()
{
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Gunner/Gunner.obj"));
    CollisionCapsule->SetOverlapCheck(false);

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
    SetCharacterMeshCount(1);
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

    if (!LastWarUI::bShowGameOver && Health <= 0)
    {
        EnableInput(Cast<APlayerController>(Controller));
        Health = 100.0f;
    }

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Bind input actions and axes here  
    //PlayerInputComponent->BindAxis("MoveForward", [this](float Value) { MoveForward(Value); });
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
    if (Controller && Value != 0.0f)
    {
        AddMovementInput(FVector::RightVector, Value);

        FVector Location = GetActorLocation();
        if (Location.Y > 14.0f)
            Location.Y = 14.0f;
        else if (Location.Y < -16.0f)
            Location.Y = -16.0f;

        SetActorLocation(Location);
    }
}

void APlayerCharacter::HandleOverlap(AActor* OtherActor)
{
    if (IsActorBeingDestroyed())  
    {  
        return;  
    }

    UE_LOG(LogLevel::Display, "Handle Overlap %s,  %s", GetData(OtherActor->GetName()), GetData(GetName()));
    
    if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor))  
    {  
        if (LuaScriptComponent)
        {
            LuaScriptComponent->ActivateFunction("OnOverlapEnemy", Enemy);
        }
    }
    else if (AWall* Wall = Cast<AWall>(OtherActor))
    {
        if (LuaScriptComponent)
        {
            if(Wall->BoxComponent)
            {
                if (!Wall->BoxComponent->GetOverlapCheck())
                {
                    return;
                }
                Wall->BoxComponent->SetOverlapCheck(false);
            }
            LuaScriptComponent->ActivateFunction("OnOverlapWall", OtherActor, Wall->GetVarientValue());
        }

        AudioManager::Get().PlayOneShot(EAudioType::Goofy);
    }

    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
        DisableInput(Cast<APlayerController>(Controller));
    }
}

// PlayerCharacter.cpp
AActor* APlayerCharacter::SpawnActorLua(const std::string& ClassName, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    UClass* SpawnClass = UClass::FindClass(ClassName.c_str());
    if (!SpawnClass)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: Cannot find class '%s'"), ClassName);
        return nullptr;
    }

    AActor* NewActor = World->SpawnActor(SpawnClass);
    if (!NewActor)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: SpawnActor returned null for '%s'"), ClassName);
        return nullptr;
    }

    NewActor->SetActorLocation(Location);

    return NewActor;
}


void APlayerCharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayerCharacter, sol::bases<AActor, APawn, ACharacter>(),
        "SpawnActorLua", &ThisClass::SpawnActorLua,
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation),
        "Health", sol::property(&ThisClass::GetHealth, &ThisClass::SetHealth),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "AttackDamage", sol::property(&ThisClass::GetAttackDamage, &ThisClass::SetAttackDamage),
        "AddCharacterMeshCount", &ThisClass::AddCharacterMeshCount
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

void APlayerCharacter::AddCharacterMeshCount(int32 InCount)
{
    CharacterMeshCount += InCount;
    CharacterMeshCount = FMath::Max(0, CharacterMeshCount);
    SetCharacterMeshCount(CharacterMeshCount);
}

void APlayerCharacter::SetCharacterMeshCount(int32 InCount)
{
    CharacterMeshCount = InCount;
    CharacterMeshCount = FMath::Max(0, CharacterMeshCount);
    
    while (StaticMeshComponents.Num() < CharacterMeshCount)
    {
        UCapsuleComponent* CapsuleComponent = AddComponent<UCapsuleComponent>();
        CapsuleComponents.Add(CapsuleComponent);
        CapsuleComponent->SetupAttachment(RootComponent);
        
        UStaticMeshComponent* StaticMeshComponent = AddComponent<UStaticMeshComponent>();
        StaticMeshComponents.Add(StaticMeshComponent);
        StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Gunner/Gunner.obj"));
        StaticMeshComponent->SetupAttachment(CapsuleComponent);
    }

    while (StaticMeshComponents.Num() > CharacterMeshCount)
    {
        UCapsuleComponent* CapsuleComponent = CapsuleComponents[CapsuleComponents.Num() - 1];
        UStaticMeshComponent* StaticMeshComponent = StaticMeshComponents[StaticMeshComponents.Num() - 1];

        CapsuleComponents.RemoveAt(CapsuleComponents.Num() - 1);
        StaticMeshComponents.RemoveAt(StaticMeshComponents.Num() - 1);
        

        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            if (EditorEngine->GetSelectedComponent() == CapsuleComponent)
            {
                EditorEngine->DeselectComponent(CapsuleComponent);
            }
            else if (EditorEngine->GetSelectedComponent() == StaticMeshComponent)
            {
                EditorEngine->DeselectComponent(StaticMeshComponent);
            }
        }
        
        // 순서 조심
        StaticMeshComponent->DestroyComponent();
        CapsuleComponent->DestroyComponent();
    }


    for (int i = 0; i < CapsuleComponents.Num(); i++)
    {
        constexpr float weight = .4f;
        float distance = FMath::Sqrt(static_cast<float>(i)) * weight;
        float cos = FMath::Cos(i * 100.f) * distance;
        float sin = FMath::Sin(i * 100.f) * distance;
        CapsuleComponents[i]->SetRelativeLocation(FVector(cos, sin, CapsuleComponents[i]->GetRelativeLocation().Z));
    }    
}
 
