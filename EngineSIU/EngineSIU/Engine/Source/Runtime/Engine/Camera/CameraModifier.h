#pragma once

#include "UObject/Object.h"



class APlayerCameraManager;

struct FMinimalViewInfo;

class UCameraModifier : public UObject
{
public:
    UCameraModifier() = default;

    virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV);

private:
    APlayerCameraManager* CameraOwner;
    float AlphaInTime;
    float AlphaOutTime;
    float Alpha;
    uint32 bDisabled;
    uint8 Priority;
};

