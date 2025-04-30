#include "CameraComponent.h"
#include "Math/JungleMath.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

void UCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    if (bUsePawnControlRotation)
    {
        const APawn* OwningPawn = Cast<APawn>(GetOwner());
        const AController* OwningController = OwningPawn ? OwningPawn->GetController() : nullptr;
        if (OwningController)
        {
            const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
            if (!PawnViewRotation.Equals(GetWorldRotation()))
            {
                SetWorldRotation(PawnViewRotation);
            }
        }
    }

    DesiredView.Location = GetRelativeLocation();
    DesiredView.Rotation = GetRelativeRotation();

    DesiredView.FOV = ViewFOV;
    DesiredView.AspectRatio = AspectRatio;
    //DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
    //DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
    DesiredView.ProjectionMode = ProjectionMode;
    DesiredView.OrthoWidth = OrthoWidth;
    DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
    DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
    //DesiredView.bAutoCalculateOrthoPlanes = bAutoCalculateOrthoPlanes;
    //DesiredView.AutoPlaneShift = AutoPlaneShift;
    //DesiredView.bUpdateOrthoPlanes = bUpdateOrthoPlanes;
    //DesiredView.bUseCameraHeightAsViewTarget = bUseCameraHeightAsViewTarget;

}
