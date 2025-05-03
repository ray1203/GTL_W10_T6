#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Components/Material/Material.h"
#include "Define.h"

class USkeletalMesh : public UObject
{
    DECLARE_CLASS(USkeletalMesh, UObject)

public:
    USkeletalMesh() = default;
    virtual ~USkeletalMesh() override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    const TArray<FStaticMaterial*>& GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& Out) const;
    OBJ::FStaticMeshRenderData* GetRenderData() const { return staticMeshRenderData; }

    //ObjectName은 경로까지 포함
    FWString GetOjbectName() const
    {
        return staticMeshRenderData->ObjectName;
    }

    void SetData(OBJ::FStaticMeshRenderData* renderData);

private:
    OBJ::FStaticMeshRenderData* staticMeshRenderData = nullptr;
    TArray<FStaticMaterial*> materials;
};
