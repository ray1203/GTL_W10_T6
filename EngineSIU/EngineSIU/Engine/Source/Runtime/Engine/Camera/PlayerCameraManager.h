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

/**
 * Cached camera POV info, stored as optimization so we only
 * need to do a full camera update once per tick.
 */
struct FCameraCacheEntry
{
    /** World time this entry was created. */
    float TimeStamp;

    /** Camera POV to cache. */
    FMinimalViewInfo POV;

    FCameraCacheEntry()
        : TimeStamp(0.f)
    {
    }
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

    virtual void UpdateCamera(float DeltaTime);

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

public:
    virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV);

    
protected:
    TArray<UCameraModifier*> ModifierList;

public:
    virtual void SetCameraCachePOV(const FMinimalViewInfo& InPOV);
    virtual void SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV);

    virtual const FMinimalViewInfo& GetCameraCacheView() const;
    virtual const FMinimalViewInfo& GetLastFrameCameraCacheView() const;

    virtual FMinimalViewInfo GetCameraCachePOV() const;
    virtual FMinimalViewInfo GetLastFrameCameraCachePOV() const;

    float GetCameraCacheTime() const { return CameraCachePrivate.TimeStamp; }
    float GetLastFrameCameraCacheTime() const { return LastFrameCameraCachePrivate.TimeStamp; }

protected:
    /** Get value of CameraCachePrivate.Time  */
    void SetCameraCacheTime(float InTime) { CameraCachePrivate.TimeStamp = InTime; }

    /** Get value of LastFrameCameraCachePrivate.Time  */
    void SetLastFrameCameraCacheTime(float InTime) { LastFrameCameraCachePrivate.TimeStamp = InTime; }

private:
    FCameraCacheEntry CameraCachePrivate;
    /** Cached camera properties, one frame old. */
    FCameraCacheEntry LastFrameCameraCachePrivate;

public:
    void FillCameraCache(const FMinimalViewInfo& NewInfo);


};

