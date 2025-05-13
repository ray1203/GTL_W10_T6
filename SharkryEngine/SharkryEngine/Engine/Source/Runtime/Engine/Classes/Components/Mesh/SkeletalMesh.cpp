#include "SkeletalMesh.h"

#include "AssetImporter/FBX/FBXManager.h"
#include "AssetImporter/FBX/FBXStructs.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/Casts.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
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

    if (SkeletalMeshRenderData->SharedVertexBuffer)
    {
        SkeletalMeshRenderData->SharedVertexBuffer->Release();
        SkeletalMeshRenderData->SharedVertexBuffer = nullptr;
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

// 회전만 변경하는 새로운 함수 추가
bool USkeletalMesh::SetBoneRotation(uint32 BoneIndex, const FMatrix& RotationMatrix)
{
    if (!Skeleton || !Skeleton->BoneTree.IsValidIndex(BoneIndex))
    {
        return false;
    }
    
    // 기존 행렬에서 이동 성분을 추출
    FVector Translation = Skeleton->CurrentPose.LocalTransforms[BoneIndex].GetTranslationVector();
    FVector Scale = Skeleton->CurrentPose.LocalTransforms[BoneIndex].GetScaleVector();
    
    // 회전 행렬에서 회전 성분만 추출하여 사용
    FMatrix RotationOnly = RotationMatrix;
    RotationOnly.RemoveTranslation(); // 이동 성분 제거
    
    // 기존 위치와 새 회전을 합쳐서 새 변환 행렬 생성
    FMatrix NewTransform = FMatrix::Identity;
    
    // 스케일 적용
    NewTransform.M[0][0] = Scale.X;
    NewTransform.M[1][1] = Scale.Y;
    NewTransform.M[2][2] = Scale.Z;
    
    // 회전 적용 (회전 행렬과 스케일 행렬 곱)
    NewTransform = RotationOnly * NewTransform;
    
    // 이동 벡터 적용
    NewTransform.M[3][0] = Translation.X;
    NewTransform.M[3][1] = Translation.Y;
    NewTransform.M[3][2] = Translation.Z;
    
    // 새 행렬로 본 업데이트
    return SetBoneLocalMatrix(BoneIndex, NewTransform);
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
    ID3D11Buffer* SharedVertexBuffer = FEngineLoop::Renderer.CreateImmutableVertexBuffer(
        SkeletalMeshRenderData->MeshName,
        SkeletalMeshRenderData->BindPoseVertices);
    SkeletalMeshRenderData->SharedVertexBuffer = SharedVertexBuffer;

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
