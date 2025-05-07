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

    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    // 카메라의 위치와 회전 설정
    float GetFieldOfView() const { return FieldOfView; }
    float GetAspectRatio() const { return AspectRatio; }
    float GetNearClip() const { return NearClip; }
    float GetFarClip() const { return FarClip; }
    void SetFieldOfView(float InFOV) { FieldOfView = InFOV; }
    void SetAspectRatio(float InAspectRatio) { AspectRatio = InAspectRatio; }
    void SetNearClip(float InNearClip) { NearClip = InNearClip; }
    void SetFarClip(float InFarClip) { FarClip = InFarClip; }

    float GetOrthoZoom() const { return OrthoZoom; }
    void SetOrthoZoom(float InOrthoZoom) { assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM); OrthoZoom = InOrthoZoom; }

    // 카메라의 프로젝션 모드 설정
    void SetProjectionMode(ECameraProjectionMode InProjectionMode){ ProjectionMode = InProjectionMode; }
    ECameraProjectionMode GetProjectionMode() const { return ProjectionMode; }

    float GetCameraCurveValue(float t);
    
    virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

    float GetOrthoWidth() const { return OrthoWidth; }
    void SetOrthoWidth(float InOrthoWidth) { OrthoWidth = InOrthoWidth; }

    float GetOrthoNearClipPlane() const { return OrthoNearClipPlane; }
    void SetOrthoNearClipPlane(float InOrthoNearClipPlane) { OrthoNearClipPlane = InOrthoNearClipPlane; }

    float GetOrthoFarClipPlane() const { return OrthoFarClipPlane; }
    void SetOrthoFarClipPlane(float InOrthoFarClipPlane) { OrthoFarClipPlane = InOrthoFarClipPlane; }

    void SetCurvePath(const FString& InCurvePath) { CurvePath = InCurvePath; }
    FString GetCurvePath() { return CurvePath; }
    
private:
    // 카메라 정보 
    float FieldOfView = 90.0f;
    float AspectRatio;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    float OrthoZoom;
    float OrthoWidth = 1536.0f;
    float OrthoNearClipPlane = -1536.0f / 2.0f;
    float OrthoFarClipPlane = 2097152.0f;
    
    ECameraProjectionMode ProjectionMode = ECameraProjectionMode::Perspective;

    FString CurvePath;
    
public:
    bool bUsePawnControlRotation = false;
};
