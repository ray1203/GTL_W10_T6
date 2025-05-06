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
FMatrix USkeletalMesh::GetBoneBindLocalMatrix(uint32 BoneIndex) const
{
    if (!Skeleton || !Skeleton->BoneTree.IsValidIndex(BoneIndex))
    {
        return FMatrix::Identity;
    }
    return Skeleton->BoneTree[BoneIndex].BindTransform;
}
bool USkeletalMesh::SetBoneLocalMatrix(uint32 BoneIndex, const FMatrix& NewLocalMatrix)
{
    if (!Skeleton || !Skeleton->BoneTree.IsValidIndex(BoneIndex))
    {
        return false;
    }
    Skeleton->CurrentPose.LocalTransforms[BoneIndex] = NewLocalMatrix;

    return true;
}

void USkeletalMesh::UpdateWorldTransforms()
{
    if (!Skeleton || Skeleton->BoneTree.IsEmpty()) return;

    // 계층 구조 순서대로 처리 (루트부터 자식으로)
    TArray<int32> ProcessingOrder; ProcessingOrder.Reserve(Skeleton->BoneTree.Num());
    TArray<uint8> Processed; Processed.Init(false, Skeleton->BoneTree.Num());
    TArray<int32> Queue; Queue.Reserve(Skeleton->BoneTree.Num());

    for (int32 i = 0; i < Skeleton->BoneTree.Num(); ++i)
        if (Skeleton->BoneTree[i].ParentIndex == INDEX_NONE)
            Queue.Add(i);

    int32 Head = 0;
    while (Head < Queue.Num())
    {
        int32 CurrentIndex = Queue[Head++];
        if (Processed[CurrentIndex]) continue;

        ProcessingOrder.Add(CurrentIndex);
        Processed[CurrentIndex] = true;

        for (int32 i = 0; i < Skeleton->BoneTree.Num(); ++i)
            if (Skeleton->BoneTree[i].ParentIndex == CurrentIndex && !Processed[i])
                Queue.Add(i);
    }

    // 오류 처리: 모든 본이 처리되었는지 확인
    if (ProcessingOrder.Num() != Skeleton->BoneTree.Num())
        for (int32 i = 0; i < Skeleton->BoneTree.Num(); ++i)
            if (!Processed[i])
                ProcessingOrder.Add(i);

    // 순서대로 월드 변환 계산
    for (int32 BoneIndex : ProcessingOrder)
    {
        int32 ParentIdx = Skeleton->BoneTree[BoneIndex].ParentIndex;

        if (ParentIdx != INDEX_NONE && Skeleton->BoneTree.IsValidIndex(ParentIdx))
        {
            // 자식 월드 = 부모 월드 * 자식 로컬 (올바른 순서)
            Skeleton->CurrentPose.GlobalTransforms[BoneIndex] =
                Skeleton->CurrentPose.LocalTransforms[BoneIndex] *
                Skeleton->CurrentPose.GlobalTransforms[ParentIdx];

            // Skeleton->CurrentPose.[BoneIndex] =
            //     Skeleton->CurrentPose.GlobalTransforms[ParentIdx] *
            //     Skeleton->CurrentPose.LocalTransforms[BoneIndex];
        }
        else
        {
            // 루트 월드 = 루트 로컬
            Skeleton->CurrentPose.GlobalTransforms[BoneIndex] =
                Skeleton->CurrentPose.LocalTransforms[BoneIndex];
        }

        // 스키닝 행렬 업데이트
        //Skeleton->UpdateCurrentPose(Skeleton->CurrentPose.LocalTransforms);
        Skeleton->CurrentPose.SkinningMatrices[BoneIndex] =
            Skeleton->CalculateSkinningMatrix(BoneIndex, Skeleton->CurrentPose.GlobalTransforms[BoneIndex]);
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
