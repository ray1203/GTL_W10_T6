#pragma once
#include "Camera/CameraModifier.h"
#include "Math/Interpolator.h"
#include "UObject/ObjectFactory.h"

struct FMinimalViewInfo;
class APlayerCameraManager;
struct Oscillator
{
    float Amplitude = 0.0f;
    float Frequency = 0.0f;
};

class UCameraShakeModifier : public UCameraModifier
{
    DECLARE_CLASS(UCameraShakeModifier, UCameraModifier)

public:
    UCameraShakeModifier();

    virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

    void SetDuration(float InDuration) { Duration = InDuration; }
    void SetBlendInTime(float InBlendInTime) { BlendInTime = InBlendInTime; }
    void SetBlendOutTime(float InBlendOutTime) { BlendOutTime = InBlendOutTime; }
    void SetScale(float InScale) { Scale = InScale; }
    float GetDuration() const { return Duration; }
    float GetBlendInTime() const { return BlendInTime; }
    float GetBlendOutTime() const { return BlendOutTime; }
    float GetScale() const { return Scale; }
    void SetInCurve(std::shared_ptr<IInterpolator> InCurve) { this->InCurve = InCurve; }
    void SetOutCurve(std::shared_ptr<IInterpolator> OutCurve) { this->OutCurve = OutCurve; }

private:
    float Duration = 0.5f;
    float BlendInTime = 0.1f;
    float BlendOutTime = 0.1f;
    float ElapsedTime = 0.0f;
    float Scale = 1.0f;

    // 축별 위치/회전 Oscillation
    Oscillator LocX, LocY, LocZ;
    Oscillator RotPitch, RotYaw, RotRoll;

    // 랜덤 위상(Phase) 부여
    float PhaseX, PhaseY, PhaseZ;
    float PhasePitch, PhaseYaw, PhaseRoll;

    std::shared_ptr<IInterpolator> InCurve;
    std::shared_ptr<IInterpolator> OutCurve;

    FVector   OriginalLocation;
    FRotator  OriginalRotation;
    bool      bHasCapturedBase = false;

};

