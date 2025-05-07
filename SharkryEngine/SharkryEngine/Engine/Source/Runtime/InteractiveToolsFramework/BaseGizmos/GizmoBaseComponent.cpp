#include "GizmoBaseComponent.h"

#include "TransformGizmo.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "World/World.h"

void UGizmoBaseComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    ATransformGizmo* TransformGizmo = Cast<ATransformGizmo>(GetOwner());
    if (TransformGizmo == nullptr)
    {
        return;
    }

    if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
    {
        return;
    }
    
    if (FEditorViewportClient* ViewportClient = TransformGizmo->GetAttachedViewport())
    {
        if (ViewportClient->IsPerspective())
        {
            float Scaler = (ViewportClient->GetPerspectiveCamera().GetLocation() - TransformGizmo->GetActorLocation()).Length();
            
            Scaler *= GizmoScale;
            RelativeScale3D = FVector(Scaler);
        }
        else
        {
            float Scaler = FEditorViewportClient::GetOrthoSize() * GizmoScale;
            RelativeScale3D = FVector(Scaler);
        }
    }
}
