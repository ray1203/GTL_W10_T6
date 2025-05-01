#include "CameraComponent.h"
#include "ImGUI/imgui.h"
#include "Engine/CurveManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

void UCameraComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    USceneComponent::GetProperties(OutProperties);
    OutProperties.Add(TEXT("FieldOfView"), FString::Printf(TEXT("%f"), FieldOfView));
    OutProperties.Add(TEXT("AspectRatio"), FString::Printf(TEXT("%f"), AspectRatio));
    OutProperties.Add(TEXT("NearClip"), FString::Printf(TEXT("%f"), NearClip));
    OutProperties.Add(TEXT("FarClip"), FString::Printf(TEXT("%f"), FarClip));
    OutProperties.Add(TEXT("OrthoZoom"), FString::Printf(TEXT("%f"), OrthoZoom));
    OutProperties.Add(TEXT("OrthoWidth"), FString::Printf(TEXT("%f"), OrthoWidth));
    OutProperties.Add(TEXT("OrthoNearClipPlane"), FString::Printf(TEXT("%f"), OrthoNearClipPlane));
    OutProperties.Add(TEXT("OrthoFarClipPlane"), FString::Printf(TEXT("%f"), OrthoFarClipPlane));
    OutProperties.Add(TEXT("ProjectionMode"), FString::Printf(TEXT("%d"), static_cast<int>(ProjectionMode)));
    OutProperties.Add(TEXT("CurvePath"), FString::Printf(TEXT("%s"), GetData(CurvePath)));
}

void UCameraComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    USceneComponent::SetProperties(InProperties);

    const FString* TempStr = nullptr;
    
    TempStr = InProperties.Find(TEXT("FieldOfView"));
    if (TempStr)
    {
        FieldOfView = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("AspectRatio"));
    if (TempStr)
    {
        AspectRatio = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("NearClip"));
    if (TempStr)
    {
        NearClip = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FarClip"));
    if (TempStr)
    {
        FarClip = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("OrthoZoom"));
    if (TempStr)
    {
        OrthoZoom = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("OrthoWidth"));
    if (TempStr)
    {
        OrthoWidth = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("OrthoNearClipPlane"));
    if (TempStr)
    {
        OrthoNearClipPlane = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("OrthoFarClipPlane"));
    if (TempStr)
    {
        OrthoFarClipPlane = FString::ToFloat(*TempStr);
    }

    TempStr = InProperties.Find(TEXT("OrthoFarClipPlane"));
    if (TempStr)
    {
        OrthoFarClipPlane = FString::ToFloat(*TempStr);
    }

    TempStr = InProperties.Find(TEXT("ProjectionMode"));
    if (TempStr)
    {
        ProjectionMode = static_cast<ECameraProjectionMode>(FString::ToInt(*TempStr));
    }

    TempStr = InProperties.Find(TEXT("CurvePath"));
    if (TempStr)
    {
        CurvePath = *TempStr;
    }
}

float UCameraComponent::GetCameraCurveValue(float t)
{
    ImVec2 Curves[100];
    CurveManager::LoadCurve(std::filesystem::path(GetData(CurvePath)), 100, Curves);
    return CurveManager::CurveValue(t, 100, Curves);
}

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

    DesiredView.Location = GetWorldLocation();
    DesiredView.Rotation = GetWorldRotation();

    DesiredView.FOV = FieldOfView;
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
