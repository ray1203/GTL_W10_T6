#include "PlayerCameraManager.h"

#include "World/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"

#include "CameraModifier.h"

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

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
    // 카메라 업데이트 로직을 여기에 추가합니다.
    // 예를 들어, 카메라의 위치나 회전을 업데이트할 수 있습니다.
    // 이 함수는 매 프레임마다 호출됩니다.


}

void APlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    for (UCameraModifier* Modifier : ModifierList)
    {
        if (Modifier)
        {
            Modifier->ModifyCamera(DeltaTime, InOutPOV);
        }
    }
}

void APlayerCameraManager::SetCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    CameraCachePrivate.POV = InPOV;
}

void APlayerCameraManager::SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    LastFrameCameraCachePrivate.POV = InPOV;
}

const FMinimalViewInfo& APlayerCameraManager::GetCameraCacheView() const
{
    return CameraCachePrivate.POV;
}

const FMinimalViewInfo& APlayerCameraManager::GetLastFrameCameraCacheView() const
{
    return LastFrameCameraCachePrivate.POV;
}

FMinimalViewInfo APlayerCameraManager::GetCameraCachePOV() const
{
    return GetCameraCacheView();
}

FMinimalViewInfo APlayerCameraManager::GetLastFrameCameraCachePOV() const
{
    return GetLastFrameCameraCacheView();
}

void APlayerCameraManager::FillCameraCache(const FMinimalViewInfo& NewInfo)
{
    // Backup last frame results.
    const float CurrentCacheTime = GetCameraCacheTime();
    const float CurrentGameTime = GetWorld()->TimeSeconds;
    if (CurrentCacheTime != CurrentGameTime)
    {
        SetLastFrameCameraCachePOV(GetCameraCacheView());
        SetLastFrameCameraCacheTime(CurrentCacheTime);
    }

    SetCameraCachePOV(NewInfo);
    SetCameraCacheTime(CurrentGameTime);
}

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color)
{
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
    FadeTime = Duration;
    FadeTimeRemaining = Duration;
    FadeColor = Color;
    FadeAmount = FromAlpha;
}
