#include "EnemyCharacter.h"
#include "Components/InputComponent.h"  
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Delegates/DelegateCombination.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

AEnemyCharacter::AEnemyCharacter()
{
    BodyMesh = AddComponent<UStaticMeshComponent>("Enemy");
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    BodyMesh->SetupAttachment(RootComponent);

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

void AEnemyCharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AEnemyCharacter, ACharacter,
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation),
        "Health", sol::property(&ThisClass::GetHealth, &ThisClass::SetHealth),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "Damage", sol::property(&ThisClass::GetDamage, &ThisClass::SetDamage)
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

    return true;
}

