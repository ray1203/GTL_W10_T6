#include "CameraShakeModifier.h"
#include "Camera/CameraTypes.h"

UCameraShakeModifier::UCameraShakeModifier()
{
    // 기본 진폭·빈도 세팅
    LocX.Amplitude = 0.0f;   LocX.Frequency = 5.0f;
    LocY.Amplitude = 0.1f;   LocY.Frequency = 5.0f;
    LocZ.Amplitude = 0.1f;   LocZ.Frequency = 5.0f;

    RotPitch.Amplitude = 0.01f;  RotPitch.Frequency = 5.0f;
    RotYaw.Amplitude = 0.01f;  RotYaw.Frequency = 5.0f;
    RotRoll.Amplitude = 0.0f;  RotRoll.Frequency = 5.0f;

    // 0 ~ 2π 사이에서 랜덤하게 위상 설정
    PhaseX = FMath::FRandRange(0.f, 2 * PI);
    PhaseY = FMath::FRandRange(0.f, 2 * PI);
    PhaseZ = FMath::FRandRange(0.f, 2 * PI);

    PhasePitch = FMath::FRandRange(0.f, 2 * PI);
    PhaseYaw = FMath::FRandRange(0.f, 2 * PI);
    PhaseRoll = FMath::FRandRange(0.f, 2 * PI);

    // 초기 시간은 0
    ElapsedTime = 0.f;
}

bool UCameraShakeModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    if (!bHasCapturedBase)
    {
        OriginalLocation = InOutPOV.Location;
        OriginalRotation = InOutPOV.Rotation;
        bHasCapturedBase = true;
    }

    ElapsedTime = FMath::Min(ElapsedTime + DeltaTime, Duration);
    if (ElapsedTime >= Duration)
        return false;  // Modifier 제거

    // 0~1 정규화
    float inT = FMath::Clamp(ElapsedTime / BlendInTime, 0.f, 1.f);
    float outT = FMath::Clamp((Duration - ElapsedTime) / BlendOutTime, 0.f, 1.f);
    float blendAlpha = InCurve ? InCurve->Evaluate(inT) : inT;
    blendAlpha *= OutCurve ? OutCurve->Evaluate(outT) : outT;
    blendAlpha *= Scale;

    auto calc = [&](const Oscillator& O, float phase)
        {
            return O.Amplitude * blendAlpha
                * FMath::Sin(phase + ElapsedTime * O.Frequency);
        };

    InOutPOV.Location = OriginalLocation + FVector(calc(LocX, PhaseX),
        calc(LocY, PhaseY),
        calc(LocZ, PhaseZ));

    InOutPOV.Rotation = OriginalRotation + FRotator(
        calc(RotPitch, PhasePitch),
        calc(RotYaw, PhaseYaw),
        calc(RotRoll, PhaseRoll)
    );

    return true;

}
