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
    void SetProjectionMode(ECameraProjectionMode InProjectionMode){ ProjectionMode = InProjectionMode; }
    ECameraProjectionMode GetProjectionMode() const { return ProjectionMode; }

    virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

private:
    // 카메라 정보 
    float ViewFOV = 90.0f;
    float AspectRatio;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    float OrthoZoom;
    float OrthoWidth = 1536.0f;
    float OrthoNearClipPlane = -1536.0f / 2.0f;
    float OrthoFarClipPlane = 2097152.0f;


    ECameraProjectionMode ProjectionMode = ECameraProjectionMode::Perspective;


public:
    bool bUsePawnControlRotation = false;
};
