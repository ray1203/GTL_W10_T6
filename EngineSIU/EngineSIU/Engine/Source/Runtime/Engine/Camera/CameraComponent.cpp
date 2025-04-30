#include "CameraComponent.h"
#include "Math/JungleMath.h"

FVector UCameraComponent::GetForwardVector() const
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    Forward = JungleMath::FVectorRotate(Forward, ViewRotation);
    return Forward;
}

FVector UCameraComponent::GetRightVector() const
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
    Right = JungleMath::FVectorRotate(Right, ViewRotation);
    return Right;
}

FVector UCameraComponent::GetUpVector() const
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    Up = JungleMath::FVectorRotate(Up, ViewRotation);
    return Up;
}

bool UCameraComponent::IsOrthographic() const
{
    return !IsPerspective();
}

bool UCameraComponent::IsPerspective() const
{
    return true;
}
