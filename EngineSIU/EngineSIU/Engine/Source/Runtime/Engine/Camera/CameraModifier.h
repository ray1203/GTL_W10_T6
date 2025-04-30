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

public:
    APlayerCameraManager* CameraOwner;
    float AlphaInTime;
    float AlphaOutTime;
    float Alpha;
    uint32 bDisabled;
    uint8 Priority;
};

