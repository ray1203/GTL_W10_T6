#include "SkeletalMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/Casts.h"
#include "FLoaderFBX.h"
#include "Container/String.h"
#include "UObject/ObjectFactory.h"
//#include " Skeleton.h" // USkeleton 클래스 포함


USkeletalMesh::USkeletalMesh()
{
    Skeleton = FObjectFactory::ConstructObject<USkeleton>(nullptr);
}

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

int32 USkeletalMesh::GetBoneIndexByName(const FName& BoneName) const
{
    if (!Skeleton) return INDEX_NONE;
    return Skeleton->GetBoneIndex(BoneName);
}

FMatrix USkeletalMesh::GetBoneLocalMatrix(uint32 BoneIndex) const
{
    if (!Skeleton || !Skeleton->BoneTree.IsValidIndex(BoneIndex))
    {
        return FMatrix::Identity;
    }
    return Skeleton->CurrentPose.LocalTransforms[BoneIndex];
}

bool USkeletalMesh::SetBoneLocalMatrix(uint32 BoneIndex, const FMatrix& NewLocalMatrix)
{
    if (!Skeleton || !Skeleton->BoneTree.IsValidIndex(BoneIndex))
    {
        return false;
    }
    bool bChanged = false;
    for (int r = 0; r < 4; ++r) 
    {
        for (int c = 0; c < 4; ++c)
        {
            if (!FMath::IsNearlyEqual(Skeleton->CurrentPose.LocalTransforms[BoneIndex].M[r][c], NewLocalMatrix.M[r][c])) 
            {
                bChanged = true;
                break;
            }
            if (bChanged) 
                break;
        }
    }

    if (bChanged)
    {
        Skeleton->CurrentPose.LocalTransforms[BoneIndex] = NewLocalMatrix;
        Skeleton->MarkBoneAndChildrenDirty(BoneIndex); // 변경된 본과 자식들을 dirty로 표시
    }
     return true;
}

void USkeletalMesh::UpdateWorldTransforms()
{
    if (!Skeleton || Skeleton->BoneTree.IsEmpty()) return;

    // Dirty 플래그가 설정된 본이 없으면 업데이트 건너뛰기
    if (!Skeleton->CurrentPose.bAnyBoneTransformDirty)
    {
        return;
    }

    // 스켈레톤으로부터 캐시된 처리 순서 가져오기
    const TArray<int32>& CurrentProcessingOrder = Skeleton->GetProcessingOrder();

    if (CurrentProcessingOrder.IsEmpty() && !Skeleton->BoneTree.IsEmpty()) 
    {
         return;
    }

    bool bActuallyUpdatedAny = false; 
    // 실제로 업데이트된 본이 있는지 추적

    // 순서대로 월드 변환 계산
    for (int32 BoneIndex : CurrentProcessingOrder)
    {
        // 배열 접근 전 유효성 검사
        if (!Skeleton->BoneTree.IsValidIndex(BoneIndex) ||
            !Skeleton->CurrentPose.LocalTransforms.IsValidIndex(BoneIndex) ||
            !Skeleton->CurrentPose.GlobalTransforms.IsValidIndex(BoneIndex) ||
            !Skeleton->CurrentPose.SkinningMatrices.IsValidIndex(BoneIndex) ||
            !Skeleton->CurrentPose.BoneTransformDirtyFlags.IsValidIndex(BoneIndex))
        {
             continue;
        }

        // 이 본이 dirty하거나, (또는 부모가 dirty해서 자식도 업데이트해야 하는 경우 - MarkBoneAndChildrenDirty에서 처리됨)
        if (Skeleton->CurrentPose.BoneTransformDirtyFlags[BoneIndex])
        {
            bActuallyUpdatedAny = true;

            const FBoneNode& CurrentBone = Skeleton->BoneTree[BoneIndex];
            const FMatrix& LocalTransform = Skeleton->CurrentPose.LocalTransforms[BoneIndex];
            int32 ParentIdx = CurrentBone.ParentIndex;

            if (ParentIdx != INDEX_NONE)
            {
                // 부모의 GlobalTransform은 ProcessingOrder에 의해 이미 최신 상태여야 함.
                // (만약 부모가 dirty였다면 이전에 이미 업데이트 되었음)
                if (!Skeleton->CurrentPose.GlobalTransforms.IsValidIndex(ParentIdx)) 
                {
                    Skeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalTransform;
                }
                else 
                {
                    const FMatrix& ParentGlobalTransform = Skeleton->CurrentPose.GlobalTransforms[ParentIdx];
                    Skeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalTransform * ParentGlobalTransform;
                }
            }
            else
            {
                Skeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalTransform;
            }

            Skeleton->CurrentPose.SkinningMatrices[BoneIndex] =
                Skeleton->CalculateSkinningMatrix(BoneIndex, Skeleton->CurrentPose.GlobalTransforms[BoneIndex]);

            Skeleton->CurrentPose.BoneTransformDirtyFlags[BoneIndex] = false; // 업데이트 완료 후 플래그 해제
        }
    }

    if (bActuallyUpdatedAny) 
    { 
        // 실제로 업데이트된 본이 있었던 경우에만 전체 플래그 해제
        // 모든 dirty 본이 처리되었는지 확인 후 bAnyBoneTransformDirty를 false로 설정해야 함.
        // 간단하게는, 업데이트가 발생했다면 false로 설정.
        // 더 정확하게는, 모든 BoneTransformDirtyFlags가 false인지 확인 후 설정.
        // 하지만 MarkBoneAndChildrenDirty에서 bAnyBoneTransformDirty를 true로 설정했으므로,
        // 업데이트가 한 번이라도 수행되면 false로 설정해도 무방.
        bool StillDirty = false;
        for (bool DirtyFlag : Skeleton->CurrentPose.BoneTransformDirtyFlags) 
        {
            if (DirtyFlag) 
            {
                StillDirty = true;
                break;
            }
        }
        if (!StillDirty) 
        {
            Skeleton->CurrentPose.bAnyBoneTransformDirty = false;
        }
    }
}

bool USkeletalMesh::UpdateAndApplySkinning()
{
    if (!SkeletalMeshRenderData || !SkeletalMeshRenderData->DynamicVertexBuffer ||
        SkeletalMeshRenderData->BindPoseVertices.IsEmpty() || !Skeleton || Skeleton->BoneTree.IsEmpty())
    {
        return false;
    }

    ID3D11DeviceContext* DeviceContext = GEngineLoop.GraphicDevice.DeviceContext;
    // 버퍼 매핑
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = DeviceContext->Map(SkeletalMeshRenderData->DynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(hr)) return false;

    FBX::FSkeletalMeshVertex* SkinnedVertices = static_cast<FBX::FSkeletalMeshVertex*>(MappedResource.pData);
    const TArray<FBX::FSkeletalMeshVertex>& BindVertices = SkeletalMeshRenderData->BindPoseVertices;
    const int32 VertexCount = BindVertices.Num();

    // 스키닝 계산
    for (int32 i = 0; i < VertexCount; ++i)
    {
        const FBX::FSkeletalMeshVertex& BindVertex = BindVertices[i];
        FBX::FSkeletalMeshVertex& SkinnedVertex = SkinnedVertices[i];
        bool bHasSignificantWeight = false;

        for (int j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            // 가중치가 충분히 크고, 연결된 본 인덱스가 유효한지 확인
            if (BindVertex.BoneWeights[j] > KINDA_SMALL_NUMBER &&
                Skeleton->BoneTree.IsValidIndex(static_cast<int32>(BindVertex.BoneIndices[j])))
            {
                bHasSignificantWeight = true; // 유의미한 가중치 발견!
                break; // 더 이상 확인할 필요 없음
            }
        }

        if (!bHasSignificantWeight) { SkinnedVertex = BindVertex; continue; }

        FVector SkinnedPosition = FVector::ZeroVector;
        FVector SkinnedNormal = FVector::ZeroVector;
        const FVector& BindPositionVec = BindVertex.Position;
        const FVector& BindNormalVec = BindVertex.Normal;

        for (int32 j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            float Weight = BindVertex.BoneWeights[j];

            if (Weight <= KINDA_SMALL_NUMBER) continue;

            int32 BoneIndex = static_cast<int32>(BindVertex.BoneIndices[j]);

            if (!Skeleton->BoneTree.IsValidIndex(BoneIndex)) continue;

            const FMatrix& SkinMatrix = Skeleton->CurrentPose.SkinningMatrices[BoneIndex];

            SkinnedPosition += SkinMatrix.TransformPosition(BindPositionVec) * Weight;

            FMatrix NormalMatrix = SkinMatrix;
            NormalMatrix.RemoveTranslation();

            if (FMath::Abs(NormalMatrix.Determinant()) > SMALL_NUMBER)
            {
                FMatrix InvTranspose = FMatrix::Transpose(FMatrix::Inverse(NormalMatrix));

                SkinnedNormal += InvTranspose.TransformPosition(BindNormalVec) * Weight;
            }
            else { SkinnedNormal += BindNormalVec * Weight; }
        }

        SkinnedVertex.Position = SkinnedPosition;
        SkinnedVertex.Normal = SkinnedNormal.GetSafeNormal();
        SkinnedVertex.TexCoord = BindVertex.TexCoord;
        // BoneIndices/Weights는 그대로 유지
        for (int k = 0; k < MAX_BONE_INFLUENCES; ++k) {
            SkinnedVertex.BoneIndices[k] = BindVertex.BoneIndices[k];
            SkinnedVertex.BoneWeights[k] = BindVertex.BoneWeights[k];
        }
    }

    // 버퍼 언매핑
    DeviceContext->Unmap(SkeletalMeshRenderData->DynamicVertexBuffer, 0);
    return true;
}

bool USkeletalMesh::GetBoneNames(TArray<FName>& OutBoneNames) const
{
    OutBoneNames.Empty();

    if (!Skeleton || Skeleton->BoneTree.IsEmpty())
    {
        return false;
    }

    OutBoneNames.Reserve(Skeleton->BoneTree.Num());

    // Skeleton 배열을 순회하며 각 본의 이름을 OutBoneNames 배열에 추가
    for (const FBoneNode& BoneNode : Skeleton->BoneTree)
    {
        OutBoneNames.Add(BoneNode.Name);
    }

    return true;
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
    Skeleton->FinalizeBoneHierarchy();

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
