#pragma once
#include "Components/SceneComponent.h"

#define MIN_ORTHOZOOM (1.0)  // 2D ortho viewport zoom >= MIN_ORTHOZOOM
#define MAX_ORTHOZOOM (1e25)

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent() = default; 

    void SetLocation(const FVector& Position){ ViewLocation = Position; }
    void SetRotation(const FVector& Rotation){ ViewRotation = Rotation; }
    void SetLookAt(const FVector& InLookAt) { LookAt = InLookAt; }
    void SetOrthoZoom(float InOrthoZoom) { assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM); OrthoZoom = InOrthoZoom; }
    const FVector& GetLocation() const { return ViewLocation; }
    const FVector& GetRotation() const { return ViewRotation; }
    const FVector& GetLookAt() const { return LookAt; }
    float GetOrthoZoom() const { return OrthoZoom; }

    float GetFOV() const { return ViewFOV; }
    float GetAspectRatio() const { return AspectRatio; }
    float GetNearClip() const { return NearClip; }
    float GetFarClip() const { return FarClip; }
    void SetFOV(float InFOV) { ViewFOV = InFOV; }
    void SetAspectRatio(float InAspectRatio) { AspectRatio = InAspectRatio; }
    void SetNearClip(float InNearClip) { NearClip = InNearClip; }
    void SetFarClip(float InFarClip) { FarClip = InFarClip; }

    FVector GetForwardVector() const;
    FVector GetRightVector() const;
    FVector GetUpVector() const;

    bool IsOrthographic() const;
    bool IsPerspective() const;

private:
    FVector ViewLocation;
    FVector ViewRotation;
    FVector DesiredLocation;
    FVector LookAt;
    FVector StartLocation;

    float OrthoZoom;
    float OrthoSize = 10.0f;
    FVector Pivot = FVector(0.0f, 0.0f, 0.0f);

    // 카메라 정보 
    float ViewFOV = 90.0f;
    float AspectRatio;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    FMatrix View;
    FMatrix Projection;
};
