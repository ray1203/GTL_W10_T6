#include "CameraModifier.h"

bool UCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    UpdateAlpha(DeltaTime);

    if (bPendingDisable && (Alpha <= 0.0f))
    {
        DisableModifier(true);
    }
    return false;
}

void UCameraModifier::DisableModifier(bool bImmediate)
{
    if (bImmediate)
    {
        bDisabled = true;
        bPendingDisable = false;
    }
    else if (!bDisabled)
    {
        bPendingDisable = true;
    }
}

void UCameraModifier::UpdateAlpha(float DeltaTime)
{
    float const TargetAlpha = GetTargetAlpha();
    float const BlendTime = (TargetAlpha == 0.f) ? AlphaOutTime : AlphaInTime;

    // interpolate!
    if (BlendTime <= 0.f)
    {
        // no blendtime means no blending, just go directly to target alpha
        Alpha = TargetAlpha;
    }
    else if (Alpha > TargetAlpha)
    {
        // interpolate downward to target, while protecting against overshooting
        Alpha = FMath::Max<float>(Alpha - DeltaTime / BlendTime, TargetAlpha);
    }
    else
    {
        // interpolate upward to target, while protecting against overshooting
        Alpha = FMath::Min<float>(Alpha + DeltaTime / BlendTime, TargetAlpha);
    }
}

float UCameraModifier::GetTargetAlpha()
{
    return bPendingDisable ? 0.0f : 1.f;
}
