#pragma once
#include "Components/SkinnedMeshComponent.h"
#include "Mesh/SkeletalMesh.h"

class UAnimSingleNodeInstance;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

public:
    USkeletalMeshComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void TickComponent(float DeltaTime) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; };

    void SetAnimAsset(const FString& AnimName);

    //virtual uint32 GetNumMaterials() const override;
    //virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    //virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    //virtual TArray<FName> GetMaterialSlotNames() const override;
    //virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;

    //virtual int CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const override;

private:
    UAnimSingleNodeInstance* AnimInstance= nullptr;
};
