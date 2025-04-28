#include "PrimitiveComponent.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"
#include "Shapes/ShapeComponent.h"
#include "Classes/GameFramework/Actor.h"

UObject* UPrimitiveComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->AABB = AABB;
    return NewComponent;
}

void UPrimitiveComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

bool UPrimitiveComponent::IntersectRayTriangle(const FVector& RayOrigin, const FVector& RayDirection, const FVector& v0, const FVector& v1, const FVector& v2, float& OutHitDistance) const
{
    const FVector Edge1 = v1 - v0;
    const FVector Edge2 = v2 - v0;

    FVector FrayDirection = RayDirection;
    FVector h = FrayDirection.Cross(Edge2);
    float a = Edge1.Dot(h);
    
    if (fabs(a) < SMALL_NUMBER)
    {
        return false; // Ray와 삼각형이 평행한 경우
    }
    float f = 1.0f / a;
    FVector s = RayOrigin - v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }
    FVector q = s.Cross(Edge1);
    float v = f * FrayDirection.Dot(q);
    if (v < 0.0f || (u + v) > 1.0f)
    {
        return false;
    }
    float t = f * Edge2.Dot(q);
    if (t > SMALL_NUMBER)
    {
        OutHitDistance = t;
        return true;
    }
    return false;
}

bool UPrimitiveComponent::IsOverlappingActor(const AActor* Other) const
{
    for (const FOverlapInfo& Info : OverlapInfos)
    {
        if (Info.OtherActor == Other)
            return true;
    }
    return false;
}

void UPrimitiveComponent::UpdateOverlaps()
{
    PreviousOverlapInfos = OverlapInfos;
    OverlapInfos.Empty();

    AActor* Owner = GetOwner();
    for (UPrimitiveComponent* Other : TObjectRange<UPrimitiveComponent>())
    {
        if (Other->GetWorld() != Owner->GetWorld()) continue;
        if (Other == this || !Cast<UShapeComponent>(Other)) continue;
        if (Owner == Other->GetOwner()) continue;

        if (CheckOverlap(Other))
        {
            OverlapInfos.Add(FOverlapInfo(Other, Other->GetOwner()));
        }
    }
}

void UPrimitiveComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("m_Type"), m_Type);
    OutProperties.Add(TEXT("AABB_min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_max"), AABB.max.ToString());
}

void UPrimitiveComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = nullptr;

    TempStr = InProperties.Find(TEXT("m_Type"));
    if (TempStr)
    {
        this->m_Type = *TempStr;
    }

    const FString* AABBminStr = InProperties.Find(TEXT("AABB_min"));
    if (AABBminStr) AABB.min.InitFromString(*AABBminStr);

    const FString* AABBmaxStr = InProperties.Find(TEXT("AABB_max"));
    if (AABBmaxStr) AABB.max.InitFromString(*AABBmaxStr);
}

// PrimitiveComponent.cpp
void UPrimitiveComponent::ProcessOverlaps()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
        return;

    TSet<AActor*> PreviousOverlappingActors;
    TSet<AActor*> CurrentOverlappingActors;

    for (const FOverlapInfo& Info : PreviousOverlapInfos)
    {
        if (Info.OtherActor)
            PreviousOverlappingActors.Add(Info.OtherActor);
    }

    for (const FOverlapInfo& Info : OverlapInfos)
    {
        if (Info.OtherActor)
            CurrentOverlappingActors.Add(Info.OtherActor);
    }

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (!PreviousOverlappingActors.Contains(OtherActor))
        {
            if (OwnerActor && !OwnerActor->IsActorBeingDestroyed())
            {
                OwnerActor->OnActorBeginOverlap.Broadcast(OtherActor);
            }
        }
    }

    for (AActor* OtherActor : PreviousOverlappingActors)
    {
        if (!CurrentOverlappingActors.Contains(OtherActor))
        {
            if (OwnerActor && !OwnerActor->IsActorBeingDestroyed())
            {
                OwnerActor->OnActorEndOverlap.Broadcast(OtherActor);
            }
        }
    }

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (OwnerActor && !OwnerActor->IsActorBeingDestroyed())
        {
            OwnerActor->OnActorOverlap.Broadcast(OtherActor);
        }
    }
}
