#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UAnimationAsset : public UObject
{
    DECLARE_CLASS(UAnimationAsset, UObject)

public:
    UAnimationAsset();
    ~UAnimationAsset() = default;
    
    const FString& GetAssetPath() const { return AssetPath; }

    void SetAssetPath(const FString& InAssetPath){ AssetPath = InAssetPath; }

protected:
    FString AssetPath;
};

