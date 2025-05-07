#include "ShapeComponent.h"

#include "UObject/Casts.h"

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    UShapeComponent* NewComponent = Cast<UShapeComponent>(Super::Duplicate(InOuter));

    if (NewComponent)
    {
        NewComponent->ShapeColor = ShapeColor;
        NewComponent->bDrawOnlyIfSelected = bDrawOnlyIfSelected;
    }

    return NewComponent;
}

void UShapeComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Color"), FLinearColor(ShapeColor).ToString());
    OutProperties.Add(TEXT("DrawOnlySelected"), bDrawOnlyIfSelected ? TEXT("true") : TEXT("false"));
}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    
    const FString* TempStr = nullptr;

    TempStr = InProperties.Find(TEXT("Color"));
    if (TempStr)
    {
        FLinearColor SavedColor;
        SavedColor = *TempStr;
        ShapeColor = SavedColor.ToColorSRGB();
    }
    TempStr = InProperties.Find(TEXT("DrawOnlySelected"));
    if (TempStr)
    {
        bDrawOnlyIfSelected = (*TempStr == TEXT("true"));
    }
}

