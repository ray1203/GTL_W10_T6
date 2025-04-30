#include "PlayerCameraManager.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"

void FViewTarget::SetNewTarget(AActor* NewTarget)
{
    Target = NewTarget;
}

APawn* FViewTarget::GetTargetPawn() const
{
    if (APawn* Pawn = Cast<APawn>(Target))
    {
        return Pawn;
    }
    else if (AController* PC = Cast<AController>(Target))
    {
        return PC->GetPawn();
    }
    else
    {
        return nullptr;
    }
}

bool FViewTarget::Equals(const FViewTarget& Other) const
{
    return Target == Other.Target && POV.Equals(Other.POV);
}

void APlayerCameraManager::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

UObject* APlayerCameraManager::Duplicate(UObject* InOuter)
{
    APlayerCameraManager* NewCameraManager = Cast<APlayerCameraManager>(Super::Duplicate(InOuter));


    return NewCameraManager;
}

void APlayerCameraManager::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayerCameraManager, sol::bases<AActor>());



}

bool APlayerCameraManager::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
        return false;

    LuaScriptComponent->GetLuaSelfTable()["this"] = this;

    return true;
}
