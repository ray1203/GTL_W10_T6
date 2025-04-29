#pragma once

#include "GameFramework/Actor.h"

class UCameraModifier;

struct FViewTarget
{
    /// ?? 뭐 넣지.
};


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager() = default;

private:
    FLinearColor FadeColor;
    float FadeAmount;
    FVector2D FadeAlpha;
    float FadeTime;
    float FadeTimeRemaining;

    FName CameraStyle;
    struct FViewTarget ViewTarget;

    TArray<UCameraModifier*> ModifierList;


};

