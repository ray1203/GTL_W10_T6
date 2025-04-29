#pragma once

#include "UObject/Object.h"

class APlayerCameraManager;

class UCameraModifier : public UObject
{
public:
    UCameraModifier() = default;


private:
    APlayerCameraManager* CameraOwner;
    float AlphaInTime;
    float AlphaOutTime;
    float Alpha;
    uint32 bDisabled;
    uint8 Priority;


};

