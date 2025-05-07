#pragma once

#include "Components/PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)

public:
    UShapeComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;

public:
    FColor GetShapeColor() const { return ShapeColor; }
    void SetShapeColor(const FColor& InColor) { ShapeColor = InColor; }
    bool IsDrawOnlyIfSelected() const { return bDrawOnlyIfSelected; }
    void SetDrawOnlyIfSelected(bool bInDrawOnlyIfSelected) { bDrawOnlyIfSelected = bInDrawOnlyIfSelected; }

public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override { return false; }

private:
    FColor ShapeColor;
    bool bDrawOnlyIfSelected;
};

