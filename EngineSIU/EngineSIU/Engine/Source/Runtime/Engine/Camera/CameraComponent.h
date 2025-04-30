#pragma once
#include "CameraTypes.h"
#include "Components/SceneComponent.h"

#define MIN_ORTHOZOOM (1.0)  // 2D ortho viewport zoom >= MIN_ORTHOZOOM
#define MAX_ORTHOZOOM (1e25)

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent() = default; 

    // 카메라의 위치와 회전 설정
    float GetFieldOfView() const { return ViewFOV; }
    float GetAspectRatio() const { return AspectRatio; }
    float GetNearClip() const { return NearClip; }
    float GetFarClip() const { return FarClip; }
    void SetFOV(float InFOV) { ViewFOV = InFOV; }
    void SetAspectRatio(float InAspectRatio) { AspectRatio = InAspectRatio; }
    void SetNearClip(float InNearClip) { NearClip = InNearClip; }
    void SetFarClip(float InFarClip) { FarClip = InFarClip; }

    float GetOrthoZoom() const { return OrthoZoom; }
    void SetOrthoZoom(float InOrthoZoom) { assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM); OrthoZoom = InOrthoZoom; }

    // 카메라의 프로젝션 모드 설정
    void SetProjectionMode(CameraProjectionMode InProjectionMode){ ProjectionMode = InProjectionMode; }
    CameraProjectionMode GetProjectionMode() const { return ProjectionMode; }

private:
    // 카메라 정보 
    float ViewFOV = 90.0f;
    float AspectRatio;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    float OrthoZoom;

    CameraProjectionMode ProjectionMode = CameraProjectionMode::Perspective;
};
