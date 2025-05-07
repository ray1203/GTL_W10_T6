#pragma once
#include "ShapeComponent.h"
#include "Renderer/ShaderConstants.h"

struct FCollisionMath;
class UBoxComponent;
class UCapsuleComponent;

class USphereComponent : public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;

    void SetRadius(float InRadius) { SphereRadius = InRadius; }
    float GetRadius() const { return SphereRadius; }

public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override;
private:
    float SphereRadius = 0;
};

