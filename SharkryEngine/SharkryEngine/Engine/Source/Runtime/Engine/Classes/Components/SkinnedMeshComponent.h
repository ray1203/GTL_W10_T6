#pragma once
#include "Components/MeshComponent.h"


namespace FBX
{
    struct FSkeletalMeshInstanceRenderData;
}

class USkeletalMesh;


class USkinnedMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent() = default;
    ~USkinnedMeshComponent();
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void TickComponent(float DeltaTime) override;
 
    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; };

    virtual uint32 GetNumMaterials() const override;
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    virtual TArray<FName> GetMaterialSlotNames() const override;
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;

    virtual int CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const override;
    
    USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }
    void SetSkeletalMesh(USkeletalMesh* value);

    void UpdateBoneTransformAndSkinning(int32 BoneIndex, const FMatrix& NewLocalMatrix);
    void SetUseGpuSkinning(bool bEnable);
    bool IsUsingGpuSkinning() const;
    FBX::FSkeletalMeshInstanceRenderData* GetInstanceRenderData();
    bool UpdateAndApplySkinning();
protected:
    USkeletalMesh* SkeletalMesh = nullptr;
    int selectedSubMeshIndex = -1;
private:
    FBX::FSkeletalMeshInstanceRenderData* InstanceRenderData= nullptr;
    //bool bUseGpuSkinning = false;
};
