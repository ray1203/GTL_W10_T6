#include "EnemyCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Enemy/enemy.obj"));

    // Initialize properties or components here
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

void AEnemyCharacter::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Health"), std::to_string(Health));
    OutProperties.Add(TEXT("Speed"), std::to_string(Speed));
    OutProperties.Add(TEXT("Damage"), std::to_string(Damage));
}

void AEnemyCharacter::SetProperties(const TMap<FString, FString>& InProperties)
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
    TempStr = InProperties.Find(TEXT("Damage"));
    if (TempStr)
    {
        Damage = std::stof(GetData(*TempStr));
    }
}

void AEnemyCharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AEnemyCharacter, sol::bases<AActor, APawn, ACharacter>(),
        "Health", sol::property(&ThisClass::GetHealth, &ThisClass::SetHealth),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "AttackDamage", sol::property(&ThisClass::GetAttackDamage, &ThisClass::SetAttackDamage)
    );
    
}

bool AEnemyCharacter::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();
    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    LuaTable["Health"] = Health;
    return true;
}

