#include "Components/SkinnedMeshComponent.h"

#include "AssetImporter/FBX/FBXStructs.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Launch/EngineLoop.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

#include "GameFramework/Actor.h"
#include "Mesh/SkeletalMesh.h"

USkinnedMeshComponent::~USkinnedMeshComponent()
{
    if (InstanceRenderData)
    {
        if (InstanceRenderData->DynamicVertexBuffer_CPU)
        {
            InstanceRenderData->DynamicVertexBuffer_CPU->Release();
            InstanceRenderData->DynamicVertexBuffer_CPU = nullptr;
        }
        delete InstanceRenderData;
        InstanceRenderData = nullptr;
    }

}

UObject* USkinnedMeshComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->selectedSubMeshIndex = selectedSubMeshIndex;
    NewComponent->SetSkeletalMesh(SkeletalMesh);
    NewComponent->SetUseGpuSkinning(IsUsingGpuSkinning());
    return NewComponent;
}

void USkinnedMeshComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

uint32 USkinnedMeshComponent::GetNumMaterials() const
{
    if (SkeletalMesh == nullptr) return 0;

    return SkeletalMesh->GetMaterials().Num();
}

UMaterial* USkinnedMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (SkeletalMesh != nullptr)
    {
        if (OverrideMaterials[ElementIndex] != nullptr)
        {
            return OverrideMaterials[ElementIndex];
        }
    
        if (SkeletalMesh->GetMaterials().IsValidIndex(ElementIndex))
        {
            return SkeletalMesh->GetMaterials()[ElementIndex]->Material;
        }
    }
    return nullptr;
}

uint32 USkinnedMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    if (SkeletalMesh == nullptr) return -1;

    return SkeletalMesh->GetMaterialIndex(MaterialSlotName);
}

TArray<FName> USkinnedMeshComponent::GetMaterialSlotNames() const
{
    TArray<FName> MaterialNames;
    if (SkeletalMesh == nullptr) return MaterialNames;

    for (const FStaticMaterial* Material : SkeletalMesh->GetMaterials())
    {
        MaterialNames.Emplace(Material->MaterialSlotName);
    }

    return MaterialNames;
}

void USkinnedMeshComponent::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    if (SkeletalMesh == nullptr) return;
    SkeletalMesh->GetUsedMaterials(Out);
    for (int materialIndex = 0; materialIndex < GetNumMaterials(); materialIndex++)
    {
        if (OverrideMaterials[materialIndex] != nullptr)
        {
            Out[materialIndex] = OverrideMaterials[materialIndex];
        }
    }
}

int USkinnedMeshComponent::CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const
{
    if (!AABB.Intersect(InRayOrigin, InRayDirection, OutHitDistance))
    {
        return 0;
    }
    if (SkeletalMesh == nullptr)
    {
        return 0;
    }

    OutHitDistance = FLT_MAX;

    int IntersectionNum = 0;

    FBX::FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetRenderData();
    const TArray<FBX::FSkeletalMeshVertex>& Vertices = RenderData->BindPoseVertices;

    const int32 VertexNum = Vertices.Num();
    if (VertexNum == 0)
    {
        return 0;
    }

    const TArray<UINT>& Indices = RenderData->Indices;
    const int32 IndexNum = Indices.Num();
    const bool bHasIndices = (IndexNum > 0);

    int32 TriangleNum = bHasIndices ? (IndexNum / 3) : (VertexNum / 3);
    for (int32 i = 0; i < TriangleNum; i++)
    {
        int32 Idx0 = i * 3;
        int32 Idx1 = i * 3 + 1;
        int32 Idx2 = i * 3 + 2;

        if (bHasIndices)
        {
            Idx0 = Indices[Idx0];
            Idx1 = Indices[Idx1];
            Idx2 = Indices[Idx2];
        }
        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        FVector v0 = SkeletalMesh->Skeleton->GetGeometryOffsetTransform(0).TransformPosition(Vertices[Idx0].Position);
        FVector v1 = SkeletalMesh->Skeleton->GetGeometryOffsetTransform(0).TransformPosition(Vertices[Idx1].Position);
        FVector v2 = SkeletalMesh->Skeleton->GetGeometryOffsetTransform(0).TransformPosition(Vertices[Idx2].Position);

        float HitDistance = FLT_MAX;
        if (IntersectRayTriangle(InRayOrigin, InRayDirection, v0, v1, v2, HitDistance))
        {
            OutHitDistance = FMath::Min(HitDistance, OutHitDistance);
            IntersectionNum++;
        }

    }
    return IntersectionNum;
}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* value)
{
     SkeletalMesh = value;

    if (SkeletalMesh == nullptr)
    {
        OverrideMaterials.SetNum(0);
        AABB = FBoundingBox(FVector::ZeroVector, FVector::ZeroVector);

        if (InstanceRenderData)
        {
            delete InstanceRenderData;
            InstanceRenderData = nullptr;
        }

        return;
    }

    if (!InstanceRenderData)
    {
        InstanceRenderData = new FBX::FSkeletalMeshInstanceRenderData();

        if (!InstanceRenderData->bUseGpuSkinning)
        {
            InstanceRenderData->DynamicVertexBuffer_CPU =
                FEngineLoop::Renderer.CreateDynamicVertexBuffer(
                    SkeletalMesh->GetRenderData()->MeshName + "_CPU_" + FString::FromInt(GetUUID()),
                    SkeletalMesh->GetRenderData()->BindPoseVertices
                );
        }
    }

    OverrideMaterials.SetNum(SkeletalMesh->GetMaterials().Num());
    AABB = FBoundingBox(SkeletalMesh->GetRenderData()->Bounds.min, SkeletalMesh->GetRenderData()->Bounds.max);
    SkeletalMesh->UpdateWorldTransforms();

    if (!InstanceRenderData->bUseGpuSkinning)
    {
        UpdateAndApplySkinning();
    }
    else
    {
        SetUseGpuSkinning(true);
    }
}

void USkinnedMeshComponent::UpdateBoneTransformAndSkinning(int32 BoneIndex, const FMatrix& NewLocalMatrix)
{
    if (!SkeletalMesh) return;

    SkeletalMesh->SetBoneLocalMatrix(BoneIndex, NewLocalMatrix);
    SkeletalMesh->UpdateWorldTransforms();

    if (!InstanceRenderData->bUseGpuSkinning)
    {
        UpdateAndApplySkinning();
    }
}
void USkinnedMeshComponent::SetUseGpuSkinning(bool bEnable)
{
    if (InstanceRenderData == nullptr)return;
    if (InstanceRenderData->bUseGpuSkinning == bEnable)
        return;

    InstanceRenderData->bUseGpuSkinning = bEnable;

    if (InstanceRenderData->bUseGpuSkinning)
    {
    }
    else
    {
        // CPU 전환 → 바로 스킨 적용이 필요한 경우만
        if (SkeletalMesh)
        {
            UpdateAndApplySkinning();
        }
    }
}

bool USkinnedMeshComponent::IsUsingGpuSkinning() const
{
    if (InstanceRenderData == nullptr) return false;
    return InstanceRenderData->bUseGpuSkinning;
}

bool USkinnedMeshComponent::UpdateAndApplySkinning()
{
    if (!InstanceRenderData || !SkeletalMesh || !SkeletalMesh->Skeleton || SkeletalMesh->GetRenderData()->BindPoseVertices.IsEmpty())
    {
        return false;
    }
    if (!InstanceRenderData->DynamicVertexBuffer_CPU)
    {
        InstanceRenderData->DynamicVertexBuffer_CPU =
            FEngineLoop::Renderer.CreateDynamicVertexBuffer(
                SkeletalMesh->GetRenderData()->MeshName + "_CPU_" + FString::FromInt(GetUUID()),
                SkeletalMesh->GetRenderData()->BindPoseVertices
            );
    }
    ID3D11DeviceContext* DeviceContext = GEngineLoop.GraphicDevice.DeviceContext;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = DeviceContext->Map(InstanceRenderData->DynamicVertexBuffer_CPU, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(hr)) return false;

    FBX::FSkeletalMeshVertex* SkinnedVertices = static_cast<FBX::FSkeletalMeshVertex*>(MappedResource.pData);
    const TArray<FBX::FSkeletalMeshVertex>& BindVertices = SkeletalMesh->GetRenderData()->BindPoseVertices;
    const int32 VertexCount = BindVertices.Num();
    const TArray<FMatrix>& SkinMatrices = SkeletalMesh->Skeleton->CurrentPose.SkinningMatrices;

    for (int32 i = 0; i < VertexCount; ++i)
    {
        const FBX::FSkeletalMeshVertex& BindVertex = BindVertices[i];
        FBX::FSkeletalMeshVertex& SkinnedVertex = SkinnedVertices[i];

        SkinnedVertex = BindVertex;

        bool bHasInfluence = false;
        for (int32 j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            if (BindVertex.BoneWeights[j] > KINDA_SMALL_NUMBER &&
                SkeletalMesh->Skeleton->BoneTree.IsValidIndex(static_cast<int32>(BindVertex.BoneIndices[j])))
            {
                bHasInfluence = true;
                break;
            }
        }

        if (!bHasInfluence) continue;

        FVector SkinnedPosition = FVector::ZeroVector;
        FVector SkinnedNormal = FVector::ZeroVector;

        float TotalWeight = 0.0f;
        for (int32 j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            float Weight = BindVertex.BoneWeights[j];
            if (Weight <= KINDA_SMALL_NUMBER) continue;

            int32 BoneIndex = static_cast<int32>(BindVertex.BoneIndices[j]);
            if (!SkeletalMesh->Skeleton->BoneTree.IsValidIndex(BoneIndex)) continue;

            TotalWeight += Weight;

            const FMatrix& SkinMatrix = SkinMatrices[BoneIndex];
            SkinnedPosition += SkinMatrix.TransformPosition(BindVertex.Position) * Weight;

            FMatrix NormalMatrix = SkinMatrix;
            NormalMatrix.RemoveTranslation();

            if (FMath::Abs(NormalMatrix.Determinant()) > SMALL_NUMBER)
            {
                FMatrix InvTranspose = FMatrix::Transpose(FMatrix::Inverse(NormalMatrix));
                SkinnedNormal += FMatrix::TransformVector(BindVertex.Normal, InvTranspose) * Weight;
            }
            else
            {
                SkinnedNormal += BindVertex.Normal * Weight;
            }
        }

        if (TotalWeight > KINDA_SMALL_NUMBER)
        {
            if (!FMath::IsNearlyEqual(TotalWeight, 1.0f))
            {
                float InvWeight = 1.0f / TotalWeight;
                SkinnedPosition *= InvWeight;
                SkinnedNormal *= InvWeight;
            }

            SkinnedVertex.Position = SkinnedPosition;
            SkinnedVertex.Normal = SkinnedNormal.GetSafeNormal();
        }
    }

    DeviceContext->Unmap(InstanceRenderData->DynamicVertexBuffer_CPU, 0);
    return true;
}

FBX::FSkeletalMeshInstanceRenderData* USkinnedMeshComponent::GetInstanceRenderData()
{
    return InstanceRenderData;
}
