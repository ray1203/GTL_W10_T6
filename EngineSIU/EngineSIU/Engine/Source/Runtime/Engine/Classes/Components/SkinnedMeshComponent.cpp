#include "Components/SkinnedMeshComponent.h"

#include "Engine/FLoaderFBX.h"
#include "Launch/EngineLoop.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

#include "GameFramework/Actor.h"

UObject* USkinnedMeshComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->selectedSubMeshIndex = selectedSubMeshIndex;
    NewComponent->SkeletalMesh = SkeletalMesh;
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
        FVector v0 = FVector(Vertices[Idx0].Position.X, Vertices[Idx0].Position.Y, Vertices[Idx0].Position.Z);
        FVector v1 = FVector(Vertices[Idx1].Position.X, Vertices[Idx1].Position.Y, Vertices[Idx1].Position.Z);
        FVector v2 = FVector(Vertices[Idx2].Position.X, Vertices[Idx2].Position.Y, Vertices[Idx2].Position.Z);

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
    }
    else
    {
        OverrideMaterials.SetNum(value->GetMaterials().Num());
        AABB = FBoundingBox(SkeletalMesh->GetRenderData()->Bounds.min, SkeletalMesh->GetRenderData()->Bounds.max);
    }
}
