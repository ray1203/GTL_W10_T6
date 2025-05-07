#include "CapsuleComponent.h"
#include "UObject/Casts.h"
#include "Math/ShapeInfo.h"
#include "Math/CollisionMath.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/SphereComponent.h"

UCapsuleComponent::UCapsuleComponent()
{
    CapsuleHalfHeight = 1;
    CapsuleRadius = 0.5;
}
UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    UCapsuleComponent* NewComponent = Cast<UCapsuleComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->CapsuleHalfHeight = CapsuleHalfHeight;
        NewComponent->CapsuleRadius = CapsuleRadius;
    }
    return NewComponent;
}

void UCapsuleComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("CapsuleHalfHeight"));
    if (TempStr)
    {
        CapsuleHalfHeight = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("CapsuleRadius"));
    if (TempStr)
    {
        CapsuleRadius = FCString::Atof(**TempStr);
    }
}

void UCapsuleComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("CapsuleHalfHeight"), FString::SanitizeFloat(CapsuleHalfHeight));
    OutProperties.Add(TEXT("CapsuleRadius"), FString::SanitizeFloat(CapsuleRadius));
}

bool UCapsuleComponent::CheckOverlap(const UPrimitiveComponent* Other) const
{
    if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionMath::IntersectCapsuleCapsule(this->ToFCapsule(), Capsule->ToFCapsule());
    }
    else if (const USphereComponent* Sphere = Cast<USphereComponent>(Other))
    {
        return FCollisionMath::IntersectCapsuleSphere(this->ToFCapsule(), Sphere->GetWorldLocation(), Sphere->GetRadius());
    }
    else if (const UBoxComponent* Box = Cast<UBoxComponent>(Other))
    {
        return FCollisionMath::IntersectBoxCapsule(Box->GetWorldBox(), this->ToFCapsule());
    }
    return false;
}


FCapsule UCapsuleComponent::ToFCapsule() const
{
    FVector Center = GetWorldLocation();
    FVector Up = GetUpVector(); 
    float HalfHeight = CapsuleHalfHeight;
    float Radius = CapsuleRadius;
    FQuat Rotation = GetWorldRotation().ToQuaternion();

    return FCapsule(Center, Up, HalfHeight, Radius, Rotation);
}
