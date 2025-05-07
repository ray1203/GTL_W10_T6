#pragma once

#include "ShapeComponent.h"


struct FCapsule;

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)

public:
    UCapsuleComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override;

    FVector GetStartPoint() const
    {
        return GetWorldLocation() + FVector(0, 0, CapsuleHalfHeight);
    }
    FVector GetEndPoint() const
    {
        return GetWorldLocation() - FVector(0, 0, CapsuleHalfHeight);
    }
    float GetCapsuleRadius() const
    {
        return CapsuleRadius;
    }
    FCapsule ToFCapsule() const;



    float GetHalfHeight() const { return CapsuleHalfHeight; }
    void SetHalfHeight(float InHeight)
    {
        InHeight = FMath::Clamp(InHeight, CapsuleRadius, 10000.f);
        CapsuleHalfHeight = InHeight;
    }

    float GetRadius() const { return CapsuleRadius; }
    void SetRadius(float InRadius)
    {
        InRadius = FMath::Clamp(InRadius, 0.f, CapsuleHalfHeight);
        CapsuleRadius = InRadius;
    }


private:
    float CapsuleHalfHeight = 1;
    float CapsuleRadius = 1;


};

