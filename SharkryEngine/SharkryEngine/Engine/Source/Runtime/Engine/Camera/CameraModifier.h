#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class APlayerCameraManager;
struct FMinimalViewInfo;

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

public:
    UCameraModifier() = default;

    virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV);
    void UpdateAlpha(float DeltaTime);
    float GetTargetAlpha();
    virtual bool IsDisabled() const { return bDisabled; }
    virtual bool IsPendingDisable() const { return bPendingDisable; }
    virtual void DisableModifier(bool bImmediate = false);

public:
    APlayerCameraManager* CameraOwner;
    float AlphaInTime = 0.0f;
    float AlphaOutTime = 0.0f;
    float Alpha = 0.0f;
    bool bDisabled = false;
    uint8 Priority;
    bool bPendingDisable = false;
};

