#pragma once

#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Color.h"

enum class CameraProjectionMode : uint8
{
    Perspective,
    Orthographic,
};

struct FMinimalViewInfo
{
public:
    FVector Location;
    FRotator Rotation;

    // Perspective mode 일 때의 Horizontal FOV(Degree). Othographic mode 일 때는 무시됨.
    float FOV;
    
    // 다양한 종횡비를 고려하기 위한 조정 전 FOV(Degree).
    float DesiredFOV;

    // Orthographic mode 일 때의 Orthographic Width. Perspective mode 일 때는 무시됨.
    float OthoroWidth;

    /** The near plane distance of the orthographic view (in world units) */
    float OrthoNearClipPlane;

    /** The far plane distance of the orthographic view (in world units) */
    float OrthoFarClipPlane;

    // Width와 Height 비율.
    float AspectRatio;

    /** The near plane distance of the perspective view (in world units). Set to a negative value to use the default global value of GNearClippingPlane */
    float PerspectiveNearClipPlane;

    float PerspectiveFarClipPlane;

    CameraProjectionMode ProjectionMode;

    float FadeAlpha;
    FLinearColor FadeColor;

private:
    // Only used for Ortho camera auto plane calculations, tells the Near plane of the extra distance that needs to be added.
    FVector CameraToViewTarget;

public:
    FMinimalViewInfo()
        : Location(FVector::ZeroVector)
        , Rotation(FVector::ZeroVector)
        , FOV(90.0f)
        , DesiredFOV(90.0f)
        , OthoroWidth(512.0f)
        , AspectRatio(1.33333333f)
        , OrthoNearClipPlane(0.0f)
        , OrthoFarClipPlane(2097152.0)
        , PerspectiveNearClipPlane(-1.0f)
        , PerspectiveFarClipPlane(10000.0)
        , ProjectionMode(CameraProjectionMode::Perspective)
        , FadeAlpha(0.0f)
        , FadeColor(FLinearColor::Black)
    { }


    bool Equals(const FMinimalViewInfo& OtherInfo) const;

    void BlendViewInfo(FMinimalViewInfo& OtherInfo, float OtherWeight);

    void ApplyBlendWeight(float BlendWeight);

    inline void SetCameraToViewTarget(const FVector& InCameraToViewTarget)
    {
        CameraToViewTarget = InCameraToViewTarget;
    }

};
