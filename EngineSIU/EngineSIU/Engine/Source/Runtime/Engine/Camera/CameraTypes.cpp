#include "CameraTypes.h"

bool FMinimalViewInfo::Equals(const FMinimalViewInfo& OtherInfo) const
{
    return Location.Equals(OtherInfo.Location) &&
        Rotation.Equals(OtherInfo.Rotation) &&
        FOV == OtherInfo.FOV &&
        DesiredFOV == OtherInfo.DesiredFOV &&
        OthoroWidth == OtherInfo.OthoroWidth &&
        AspectRatio == OtherInfo.AspectRatio &&
        OrthoNearClipPlane == OtherInfo.OrthoNearClipPlane &&
        OrthoFarClipPlane == OtherInfo.OrthoFarClipPlane &&
        PerspectiveNearClipPlane == OtherInfo.PerspectiveNearClipPlane;
}

void FMinimalViewInfo::BlendViewInfo(FMinimalViewInfo& OtherInfo, float OtherWeight)
{
    Location = FMath::Lerp(Location, OtherInfo.Location, OtherWeight);

    FRotator DeltaAng = (OtherInfo.Rotation - Rotation).GetNormalized();
    DeltaAng *= OtherWeight;
    Rotation = Rotation + DeltaAng;

    FOV = FMath::Lerp(FOV, OtherInfo.FOV, OtherWeight);
    OthoroWidth = FMath::Lerp(OthoroWidth, OtherInfo.OthoroWidth, OtherWeight);
    OrthoNearClipPlane = FMath::Lerp(OrthoNearClipPlane, OtherInfo.OrthoNearClipPlane, OtherWeight);
    OrthoFarClipPlane = FMath::Lerp(OrthoFarClipPlane, OtherInfo.OrthoFarClipPlane, OtherWeight);
    PerspectiveNearClipPlane = FMath::Lerp(PerspectiveNearClipPlane, OtherInfo.PerspectiveNearClipPlane, OtherWeight);

    AspectRatio = FMath::Lerp(AspectRatio, OtherInfo.AspectRatio, OtherWeight);

}

void FMinimalViewInfo::ApplyBlendWeight(float BlendWeight)
{
    Location *= BlendWeight;
    Rotation.Normalize();
    Rotation *= BlendWeight;
    FOV *= BlendWeight;
    DesiredFOV *= BlendWeight;
    OthoroWidth *= BlendWeight;
    AspectRatio *= BlendWeight;
    OrthoNearClipPlane *= BlendWeight;
    OrthoFarClipPlane *= BlendWeight;
    PerspectiveNearClipPlane *= BlendWeight;
}
