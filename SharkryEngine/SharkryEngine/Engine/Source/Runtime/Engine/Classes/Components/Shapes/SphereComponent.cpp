#include "SphereComponent.h"

#include "UObject/Casts.h"
#include "Math/CollisionMath.h"
#include "Math/ShapeInfo.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/CapsuleComponent.h"

USphereComponent::USphereComponent()
{
    SphereRadius = 1;
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    USphereComponent* NewComponent = Cast<USphereComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->SphereRadius = SphereRadius;
    }
    return NewComponent;
}

bool USphereComponent::CheckOverlap(const UPrimitiveComponent* Other) const
{
    if (const USphereComponent* Sphere = Cast<USphereComponent>(Other))
    {
        // Sphere vs Sphere
        return FCollisionMath::IntersectSphereSphere(
            FSphere(this->GetWorldLocation(), SphereRadius),
            FSphere(Sphere->GetWorldLocation(), Sphere->SphereRadius)
        );
    }
    else if (const UBoxComponent* Box = Cast<UBoxComponent>(Other))
    {
        // Sphere vs Box
        return FCollisionMath::IntersectBoxSphere(
            Box->GetWorldBox(), this->GetWorldLocation(), SphereRadius
        );
    }
    else if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Other))
    {
        // Sphere vs Capsule
        return FCollisionMath::IntersectCapsuleSphere(
            Capsule->ToFCapsule(),
            this->GetWorldLocation(),
            SphereRadius
        );

    }

    return false;
}


void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("SphereRadius"));
    if (TempStr)
    {
        SphereRadius = FCString::Atof(**TempStr);
    }
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("SphereRadius"), FString::SanitizeFloat(SphereRadius));
}
