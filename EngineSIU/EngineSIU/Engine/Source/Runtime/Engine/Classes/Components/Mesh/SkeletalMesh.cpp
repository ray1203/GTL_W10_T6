#include "SkeletalMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/Casts.h"
#include "FLoaderFBX.h"
#include "Container/String.h"
#include "UObject/ObjectFactory.h"


USkeletalMesh::~USkeletalMesh()
{
    if (SkeletalMeshRenderData == nullptr) return;

     if (SkeletalMeshRenderData->DynamicVertexBuffer) 
     {
         SkeletalMeshRenderData->DynamicVertexBuffer->Release();
         SkeletalMeshRenderData->DynamicVertexBuffer = nullptr;
     }

     if (SkeletalMeshRenderData->IndexBuffer) 
     {
         SkeletalMeshRenderData->IndexBuffer->Release();
         SkeletalMeshRenderData->IndexBuffer = nullptr;
     }
}

UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{
    // TODO: Context->CopyResource를 사용해서 Buffer복사
    // ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate());
    return nullptr;
}

uint32 USkeletalMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void USkeletalMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (const FStaticMaterial* Material : materials)
    {
        Out.Emplace(Material->Material);
    }
}

FWString USkeletalMesh::GetObjectName() const
{
    if (SkeletalMeshRenderData)
    {
        return SkeletalMeshRenderData->MeshName.ToWideString();
    }
    return FWString();
}

void USkeletalMesh::SetData(FBX::FSkeletalMeshRenderData* renderData)
{
    SkeletalMeshRenderData = renderData;

    uint32 verticeNum = SkeletalMeshRenderData->BindPoseVertices.Num();
    if (verticeNum <= 0) return;
    SkeletalMeshRenderData->DynamicVertexBuffer = FEngineLoop::Renderer.CreateDynamicVertexBuffer(SkeletalMeshRenderData->MeshName, SkeletalMeshRenderData->BindPoseVertices);

    uint32 indexNum = SkeletalMeshRenderData->Indices.Num();
    if (indexNum > 0)
        SkeletalMeshRenderData->IndexBuffer = FEngineLoop::Renderer.CreateImmutableIndexBuffer(SkeletalMeshRenderData->MeshName, SkeletalMeshRenderData->Indices);

    for (int materialIndex = 0; materialIndex < SkeletalMeshRenderData->Materials.Num(); materialIndex++)
    {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        UMaterial* newMaterial = FManagerFBX::CreateMaterial(SkeletalMeshRenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = SkeletalMeshRenderData->Materials[materialIndex].MaterialName;

        materials.Add(newMaterialSlot);
    }
}
