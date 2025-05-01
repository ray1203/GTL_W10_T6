#pragma once

#include "GameFramework/Actor.h"
#include "CameraTypes.h"

class AActor;
class UCameraModifier;

enum class EViewTargetBlendFunction : int
{
    /** Camera does a simple linear interpolation. */
    VTBlend_Linear,
    /** Camera has a slight ease in and ease out, but amount of ease cannot be tweaked. */
    VTBlend_Cubic,
    /** Camera immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseIn,
    /** Camera smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseOut,
    /** Camera smoothly accelerates and decelerates.  Ease amount controlled by BlendExp. */
    VTBlend_EaseInOut,
    /** The game's camera system has already performed the blending. Engine should not blend at all */
    VTBlend_PreBlended,
    VTBlend_MAX,
};


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

    /** Make sure ViewTarget is valid */
    void CheckViewTarget(APlayerController* OwningController);
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


/** A set of parameters to describe how to transition between view targets. */
struct FViewTargetTransitionParams
{
public:

    /** Total duration of blend to pending view target. 0 means no blending. */
    float BlendTime;

    /** Function to apply to the blend parameter. */
    EViewTargetBlendFunction BlendFunction;

    /** Exponent, used by certain blend functions to control the shape of the curve. */
    float BlendExp;

    /**
     * If true, lock outgoing viewtarget to last frame's camera POV for the remainder of the blend.
     * This is useful if you plan to teleport the old viewtarget, but don't want to affect the blend.
     */
    uint32 bLockOutgoing : 1;

    FViewTargetTransitionParams()
        : BlendTime(0.1f)
        , BlendFunction(EViewTargetBlendFunction::VTBlend_Cubic)
        , BlendExp(2.f)
        , bLockOutgoing(false)
    {
    }

    /** For a given linear blend value (blend percentage), return the final blend alpha with the requested function applied */
    float GetBlendAlpha(const float& TimePct) const;
};

class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager() = default;

    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    void AddModifier(UCameraModifier* Modifier);

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    virtual void UpdateCamera(float DeltaTime);

    virtual void UpdateViewTarget(FViewTarget& OutVT, float DeltaTime);

private:
    virtual bool RemoveCameraModifier(UCameraModifier* ModifierToRemove);

private:
    class APlayerController* PCOwner = nullptr;
    
    class USceneComponent* TransformComponent = nullptr;

public:
    FLinearColor FadeColor = FLinearColor::Red;
    float FadeAmount;
    FVector2D FadeAlpha;
    float FadeTime;
    float FadeTimeRemaining;

    /** The default desired width (in world units) of the orthographic view (ignored in Perspective mode) */
    float DefaultOrthoWidth = 512.0f;
    /** Default aspect ratio. Most of the time the value from a camera component will be used instead. */
    float DefaultAspectRatio = 1.33333f;
    /** FOV to use by default. */
    float DefaultFOV = 90.0f;

    FName CameraStyle;

    // 현재 ViweTarget 정보.
    FViewTarget ViewTarget;
    // Blending을 위한 ViewTarget 정보.
    FViewTarget PendingViewTarget;

    // Blending에 쓰일 시간.
    float BlendTimeToGo = 0.0f;

    struct FViewTargetTransitionParams BlendParams;

public:
    virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

    virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV);

    virtual void InitializeFor(class APlayerController* PC);
    
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

    virtual void AssignViewTarget(AActor* NewTarget, FViewTarget& VT, struct FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

public:
    virtual void StartCameraFade(float FromAlpha, float ToAlpha, float InFadeTime, FLinearColor InFadeColor, bool bInFadeAudio, bool bInHoldWhenFinished);

    virtual void StopCameraFade();

public:
    /** True when this camera should use an orthographic perspective instead of FOV */
    bool bIsOrthographic = false;
    bool bEnableFading = false;
    bool bHoldFadeWhenFinished = false;

};

