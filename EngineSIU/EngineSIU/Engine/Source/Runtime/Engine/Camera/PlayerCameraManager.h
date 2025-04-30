#pragma once

#include "GameFramework/Actor.h"
#include "CameraTypes.h"

class AActor;
class UCameraModifier;

struct FViewTarget
{

public:
    AActor* Target;

    // 카메라 Point of view 계산용.
    FMinimalViewInfo POV;

public:
    FViewTarget()
        : Target(nullptr), POV(FMinimalViewInfo())
    {}

    void SetNewTarget(AActor* NewTarget);

    class APawn* GetTargetPawn() const;

    bool Equals(const FViewTarget& Other) const;

};


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager() = default;

    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

private:
    class APlayerController* PCOwner;
    
    class USceneComponent* TransformComponent;

public:
    FLinearColor FadeColor;
    float FadeAmount;
    FVector2D FadeAlpha;
    float FadeTime;
    float FadeTimeRemaining;

    FName CameraStyle;

    // 현재 ViweTarget 정보.
    FViewTarget ViewTarget;
    // Blending을 위한 ViewTarget 정보.
    FViewTarget PendingViewTarget;

    // Blending에 쓰일 시간.
    float BlendTimeToGo;

    
protected:
    TArray<UCameraModifier*> ModifierList;

};

