#include "EnemyCharacter.h"
#include "Components/InputComponent.h"  
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"

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

