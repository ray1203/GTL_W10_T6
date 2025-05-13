
#include "FBXMeshLoader.h"

#include "FBXConversionUtil.h"
#include "FBXSkeletonBuilder.h"
#include "Animation/Skeleton.h"
#include "FBXStructs.h"
bool FBXMeshLoader::ConvertToSkeletalMesh(const TArray<FBX::MeshRawData>& AllRawMeshData, const FBX::FBXInfo& FullFBXInfo, FBX::FSkeletalMeshRenderData& OutSkeletalMeshRenderData, USkeleton* OutSkeleton)
{
    using namespace ::FBX;

    if (!OutSkeleton) return false;


    OutSkeletalMeshRenderData.MeshName = AllRawMeshData[0].NodeName.ToString();
    OutSkeletalMeshRenderData.FilePath = FullFBXInfo.FilePath;
    OutSkeleton->Clear();

    // 1. 스키닝에 관련된 모든 본 및 그 부모 본들 수집 (BonesToInclude)
    TArray<FName> RelevantBoneNames;
    for (const auto& RawMeshDataInstance : AllRawMeshData)
    {
        // 모든 메시의 영향 본 수집
        for (const auto& influence : RawMeshDataInstance.SkinningInfluences)
        {
            RelevantBoneNames.AddUnique(influence.BoneName);
        }
    }
    if (RelevantBoneNames.IsEmpty() && !AllRawMeshData.IsEmpty() && !FullFBXInfo.SkeletonHierarchy.IsEmpty())
    {
        FullFBXInfo.SkeletonHierarchy.GetKeys(RelevantBoneNames);
    }

    TArray<FName> BonesToInclude;
    FullFBXInfo.SkeletonHierarchy.GetKeys(BonesToInclude);
    int32 CheckIndex = 0;
    while (CheckIndex < BonesToInclude.Num())
    {
        FName CurrentBoneName = BonesToInclude[CheckIndex++];
        const FBoneHierarchyNode* HNode = FullFBXInfo.SkeletonHierarchy.Find(CurrentBoneName);
        if (HNode && !HNode->ParentName.IsNone() && FullFBXInfo.SkeletonHierarchy.Contains(HNode->ParentName))
        {
            BonesToInclude.AddUnique(HNode->ParentName);
        }
    }


    // 2. 루트 본 식별 및 자식 관계 맵핑 (USkeleton에 아직 추가 안 함)
    TMap<FName, TArray<FName>> BoneChildrenMap;
    TArray<FName> RootBoneNamesForSorting;
    for (const FName& BoneName : BonesToInclude)
    {
        const FBoneHierarchyNode* HNode = FullFBXInfo.SkeletonHierarchy.Find(BoneName);
        if (HNode)
        {
            if (HNode->ParentName.IsNone() || !BonesToInclude.Contains(HNode->ParentName))
            {
                RootBoneNamesForSorting.AddUnique(BoneName);
            }
            else
            {
                BoneChildrenMap.FindOrAdd(HNode->ParentName).Add(BoneName);
            }
        }
    }

    // 3. 본들을 위상 정렬 (부모가 항상 자식보다 먼저 오도록)
    TArray<FName> SortedBoneNames; SortedBoneNames.Reserve(BonesToInclude.Num());
    TArray<FName> ProcessingQueue = RootBoneNamesForSorting;
    TMap<FName, bool> ProcessedBones;
    int32 Head = 0;
    while (Head < ProcessingQueue.Num())
    {
        FName CurrentBoneName = ProcessingQueue[Head++];
        if (ProcessedBones.Contains(CurrentBoneName))
        {
            continue;
        }
        SortedBoneNames.Add(CurrentBoneName);
        ProcessedBones.Add(CurrentBoneName, true);
        if (BoneChildrenMap.Contains(CurrentBoneName))
        {
            for (const FName& ChildBoneName : BoneChildrenMap[CurrentBoneName])
            {
                if (!ProcessedBones.Contains(ChildBoneName))
                {
                    ProcessingQueue.Add(ChildBoneName);
                }
            }
        }
    }
    if (SortedBoneNames.Num() != BonesToInclude.Num())
    {
        for (const FName& BoneName : BonesToInclude)
        {
            if (!ProcessedBones.Contains(BoneName))
            {
                SortedBoneNames.Add(BoneName);
            }
        }
    }

    // 4. 정렬된 순서대로 USkeleton에 본 추가 (단일 호출 지점)
    for (const FName& BoneName : SortedBoneNames)
    {
        const FBoneHierarchyNode* HNode = FullFBXInfo.SkeletonHierarchy.Find(BoneName);
        if (HNode)
        {
            int32 ParentIndexInSkeleton = INDEX_NONE;
            if (!HNode->ParentName.IsNone())
            {
                const uint32* FoundParentIndexPtr = OutSkeleton->BoneNameToIndex.Find(HNode->ParentName);
                if (FoundParentIndexPtr) ParentIndexInSkeleton = static_cast<int32>(*FoundParentIndexPtr);
            }
            OutSkeleton->AddBone(BoneName, ParentIndexInSkeleton, HNode->GlobalBindPose, HNode->TransformMatrix);
        }
    }

    // 2. Prepare Skinning Data
    // --- 2. 메시 데이터 통합 ---
    TArray<FVector> CombinedControlPoints_MeshNodeLocal; // 각 메시 노드 로컬 공간 기준 컨트롤 포인트
    TArray<int32> CombinedPolygonVertexIndices;
    TArray<FControlPointSkinningData> CombinedCpSkinData; // 컨트롤 포인트 기준 스키닝 데이터

    // ReconstructVertexAttributes 결과 통합
    TArray<FVector> CombinedPVNormals;
    TArray<FVector2D> CombinedPVUVs;

    uint32 GlobalVertexOffset = 0; // 전체 컨트롤 포인트에 대한 오프셋
    uint32 GlobalPolyVertOffset = 0; // 전체 폴리곤 정점에 대한 오프셋 (PVNormals, PVUVs 인덱싱용)


    for (const FBX::MeshRawData& CurrentRawMeshData : AllRawMeshData)
    {
        // 2a. 메시 노드의 글로벌 변환 가져오기 (이 변환은 정점을 월드(또는 공통) 바인드 공간으로 옮김)
        // 중요: 이 변환은 FBX 파싱 시 각 MeshRawData에 저장되어 있어야 함.
        // 여기서는 임시로 단위 행렬로 가정. 실제로는 각 메시 노드의 글로벌 변환을 사용해야 함.
        const FMatrix& MeshNodeWorldBindTransform = FMatrix::Identity;//CurrentRawMeshData.MeshNodeGlobalTransformAtBindTime; // 저장된 값 사용

        // 컨트롤 포인트 추가 (메시 노드 변환 적용)
        for (const FVector& cp_local_to_mesh_node : CurrentRawMeshData.ControlPoints)
        {
            //Transform이 적용 안된 정보 삽입(GPU Skinning용)
            CombinedControlPoints_MeshNodeLocal.Add(cp_local_to_mesh_node);
            //CombinedControlPoints_MeshNodeLocal.Add(MeshNodeWorldBindTransform.TransformPosition(cp_local_to_mesh_node));
        }

        // 스키닝 데이터 추가 (컨트롤 포인트 인덱스에 GlobalVertexOffset 적용)
        int32 CurrentMeshCPCount = CurrentRawMeshData.ControlPoints.Num();
        int32 OldSize = CombinedCpSkinData.Num();
        CombinedCpSkinData.SetNum(OldSize + CurrentMeshCPCount);

        for (const auto& influence : CurrentRawMeshData.SkinningInfluences)
        {
            const uint32* SkelBoneIndexPtr = OutSkeleton->BoneNameToIndex.Find(influence.BoneName);
            if (!SkelBoneIndexPtr) continue;
            int32 SkelBoneIndex = static_cast<int32>(*SkelBoneIndexPtr);

            for (int i = 0; i < influence.ControlPointIndices.Num(); ++i)
            {
                int32 LocalCPIndex = influence.ControlPointIndices[i];
                int32 GlobalCPIndex = GlobalVertexOffset + LocalCPIndex; // 글로벌 인덱스
                float Weight = static_cast<float>(influence.ControlPointWeights[i]);

                if (CombinedCpSkinData.IsValidIndex(GlobalCPIndex) && Weight > KINDA_SMALL_NUMBER)
                {
                    CombinedCpSkinData[GlobalCPIndex].Influences.Add({ SkelBoneIndex, Weight });
                }
            }
        }

        // 정점 속성 재구성 및 통합
        TArray<FVector> TempCPNormals, TempPVNormals_ForThisMesh;
        TArray<FVector2D> TempCPUVs, TempPVUVs_ForThisMesh;
        if (!ReconstructVertexAttributes(CurrentRawMeshData, TempCPNormals, TempPVNormals_ForThisMesh, TempCPUVs, TempPVUVs_ForThisMesh))
        {
            return false; // 또는 기본값으로 계속 진행하는 로직 필요
        }
        FMatrix NormalTransform = MeshNodeWorldBindTransform;
        NormalTransform.RemoveTranslation();

        for (const FVector& normal : TempPVNormals_ForThisMesh)
        {
            CombinedPVNormals.Add(NormalTransform.TransformPosition(normal).GetSafeNormal());
        }
        for (const FVector2D& uv : TempPVUVs_ForThisMesh)
        {
            CombinedPVUVs.Add(uv);
        }



        // 폴리곤 정점 인덱스 추가 (컨트롤 포인트 인덱스에 GlobalVertexOffset 적용)
        for (int32 local_cp_idx : CurrentRawMeshData.PolygonVertexIndices)
        {
            CombinedPolygonVertexIndices.Add(GlobalVertexOffset + local_cp_idx);
        }
        GlobalVertexOffset += CurrentMeshCPCount;
        GlobalPolyVertOffset += CurrentRawMeshData.PolygonVertexIndices.Num(); // PV 속성 인덱싱을 위함
    }

    // --- 3. 통합된 스키닝 가중치 정규화 ---
    for (int32 cpIdx = 0; cpIdx < CombinedCpSkinData.Num(); ++cpIdx)
    {
        CombinedCpSkinData[cpIdx].NormalizeWeights(MAX_BONE_INFLUENCES);
    }


    OutSkeletalMeshRenderData.BindPoseVertices.Empty();
    OutSkeletalMeshRenderData.Indices.Empty();
    std::unordered_map<FSkeletalMeshVertex, uint32> UniqueFinalVertices;
    uint32 FinalVertexIndexCounter = 0;

    for (int32 FinalPolyVertIdx = 0; FinalPolyVertIdx < CombinedPolygonVertexIndices.Num(); ++FinalPolyVertIdx)
    {
        int32 GlobalCPIndex = CombinedPolygonVertexIndices[FinalPolyVertIdx]; // 폴리곤이 참조하는 컨트롤 포인트의 글로벌 인덱스

        FSkeletalMeshVertex CurrentFinalVertex = {};

        if (CombinedControlPoints_MeshNodeLocal.IsValidIndex(GlobalCPIndex))
        {
            CurrentFinalVertex.Position = CombinedControlPoints_MeshNodeLocal[GlobalCPIndex];
        }
        else { /* 오류 처리 또는 기본값 */ CurrentFinalVertex.Position = FVector::ZeroVector; }

        if (CombinedPVNormals.IsValidIndex(FinalPolyVertIdx))
        { // PV속성은 최종 폴리곤 정점 순서를 따름
            CurrentFinalVertex.Normal = CombinedPVNormals[FinalPolyVertIdx];
        }
        else
        {
            CurrentFinalVertex.Normal = FVector(0, 0, 1);
        }
        if (!CurrentFinalVertex.Normal.IsNearlyZero())
            CurrentFinalVertex.Normal.Normalize();
        else
            CurrentFinalVertex.Normal = FVector(0, 0, 1);

        if (CombinedPVUVs.IsValidIndex(FinalPolyVertIdx))
        {
            CurrentFinalVertex.TexCoord = CombinedPVUVs[FinalPolyVertIdx];
        }
        else
        {
            CurrentFinalVertex.TexCoord = FVector2D::ZeroVector;
        }

        if (CombinedCpSkinData.IsValidIndex(GlobalCPIndex))
        {
            const auto& InfluencesForCP = CombinedCpSkinData[GlobalCPIndex].Influences;
            // NormalizeWeights가 이미 호출되었으므로, 여기서 가져온 가중치는 정규화된 상태여야 함.
            for (int32 i = 0; i < InfluencesForCP.Num() && i < MAX_BONE_INFLUENCES; ++i)
            {
                CurrentFinalVertex.BoneIndices[i] = InfluencesForCP[i].BoneIndex;
                CurrentFinalVertex.BoneWeights[i] = InfluencesForCP[i].Weight;
            }
            for (int32 i = InfluencesForCP.Num(); i < MAX_BONE_INFLUENCES; ++i)
            {
                CurrentFinalVertex.BoneIndices[i] = 0;
                CurrentFinalVertex.BoneWeights[i] = 0.0f;
            }
        }
        else
        {
            // 스키닝 정보 없는 경우, 루트 본에 100% 가중치 (또는 다른 처리)
            CurrentFinalVertex.BoneIndices[0] = 0;
            CurrentFinalVertex.BoneWeights[0] = 1.0f;
        }


        auto it = UniqueFinalVertices.find(CurrentFinalVertex);
        if (it != UniqueFinalVertices.end())
        {
            OutSkeletalMeshRenderData.Indices.Add(it->second);
        }
        else
        {
            uint32 NewIndex = FinalVertexIndexCounter++;
            OutSkeletalMeshRenderData.BindPoseVertices.Add(CurrentFinalVertex);
            UniqueFinalVertices[CurrentFinalVertex] = NewIndex;
            OutSkeletalMeshRenderData.Indices.Add(NewIndex);
        }
    }

    // 5. Populate Materials
    OutSkeletalMeshRenderData.Materials.Empty();
    TMap<FName, int32> MatNameToIndexMap;
    for (const FBX::MeshRawData& CurrentRawMeshData : AllRawMeshData)
    {
        for (const FName& MatName : CurrentRawMeshData.MaterialNames)
        {
            if (MatName.IsNone())
            {
                if (!MatNameToIndexMap.Contains(NAME_None))
                {
                    int32 DefIdx = OutSkeletalMeshRenderData.Materials.Add(FFbxMaterialInfo());
                    MatNameToIndexMap.Add(NAME_None, DefIdx);
                }
                continue;
            }

            const FFbxMaterialInfo* MatInfoPtr = FullFBXInfo.Materials.Find(MatName);
            if (MatInfoPtr && !MatNameToIndexMap.Contains(MatName))
            {
                int32 NewIdx = OutSkeletalMeshRenderData.Materials.Add(*MatInfoPtr);
                MatNameToIndexMap.Add(MatName, NewIdx);
            }
            else if (!MatInfoPtr && !MatNameToIndexMap.Contains(MatName))
            {
                int32 DefIdx = OutSkeletalMeshRenderData.Materials.Add(FFbxMaterialInfo());
                MatNameToIndexMap.Add(MatName, DefIdx);
            }
        }
    }
    if (OutSkeletalMeshRenderData.Materials.IsEmpty() && !OutSkeletalMeshRenderData.Indices.IsEmpty())
    {
        OutSkeletalMeshRenderData.Materials.Add(FFbxMaterialInfo());
        MatNameToIndexMap.Add(NAME_None, 0);
    }
    for (const FBX::MeshRawData& CurrentRawMeshData : AllRawMeshData)
    {
        if (!CreateMaterialSubsetsInternal(CurrentRawMeshData, MatNameToIndexMap, OutSkeletalMeshRenderData))
        {
            if (!OutSkeletalMeshRenderData.Indices.IsEmpty() && !OutSkeletalMeshRenderData.Materials.IsEmpty())
            {
                OutSkeletalMeshRenderData.Subsets.Empty();
                FMeshSubset DefSub;
                DefSub.MaterialIndex = 0;
                DefSub.IndexStart = 0;
                DefSub.IndexCount = OutSkeletalMeshRenderData.Indices.Num();
                OutSkeletalMeshRenderData.Subsets.Add(DefSub);
            }
            else
            {
                return false;
            }
        }
    }
    uint32 TotalSubIdx = 0;
    for (const auto& sub : OutSkeletalMeshRenderData.Subsets)
        TotalSubIdx += sub.IndexCount;
    if (TotalSubIdx != OutSkeletalMeshRenderData.Indices.Num())
        return false;

    // 7. Calculate Initial Local Transforms
    FBX::FBXSkeletonBuilder::CalculateInitialLocalTransformsInternal(OutSkeleton);

    // 8. TODO: Calculate Tangents (Requires Tangent member in FSkeletalMeshVertex and averaging logic)


    // 9. Calculate Bounding Box
    TArray<FVector> PositionsForBoundsCalculation;
    PositionsForBoundsCalculation.Reserve(OutSkeletalMeshRenderData.BindPoseVertices.Num());

    for (const FBX::FSkeletalMeshVertex& VertexToSkinForBounds : OutSkeletalMeshRenderData.BindPoseVertices)
    {
        FVector FinalPosForBounds = FVector::ZeroVector;
        bool bWasSkinned = false; // 실제로 스키닝 계산이 수행되었는지

        // VertexToSkinForBounds.Position은 이미 메시 노드의 월드 변환까지 적용된 공통 공간 기준 위치
        const FVector& BasePositionForSkining = VertexToSkinForBounds.Position;

        for (int j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            uint32 BoneIdx = VertexToSkinForBounds.BoneIndices[j];
            float Weight = VertexToSkinForBounds.BoneWeights[j];

            if (Weight <= KINDA_SMALL_NUMBER) continue;
            if (!OutSkeleton->BoneTree.IsValidIndex(BoneIdx)) continue;

            bWasSkinned = true;
            const FBoneNode& BoneNode = OutSkeleton->BoneTree[BoneIdx];
            const FMatrix& GeometryOffset = BoneNode.GeometryOffsetMatrix;

            FinalPosForBounds += GeometryOffset.TransformPosition(BasePositionForSkining) * Weight;
        }

        if (!bWasSkinned)
        {
            // 스키닝 가중치가 전혀 없는 정점 (예: 모든 가중치가 0이거나, 유효한 본 인덱스가 없음)
            FinalPosForBounds = BasePositionForSkining; // 원본 위치 (메시 노드 월드 변환 적용된 상태) 그대로 사용
        }

        PositionsForBoundsCalculation.Add(FinalPosForBounds);
    }

    TArray<FBX::FSkeletalMeshVertex> TempVerticesForBounds;

    TempVerticesForBounds.Reserve(PositionsForBoundsCalculation.Num());

    for (const FVector& Pos : PositionsForBoundsCalculation)
    {
        FBX::FSkeletalMeshVertex TempVtx;
        TempVtx.Position = Pos;
        TempVerticesForBounds.Add(TempVtx);
    }
    ComputeBoundingBox(TempVerticesForBounds, OutSkeletalMeshRenderData.Bounds.min, OutSkeletalMeshRenderData.Bounds.max);


    return true;
}
void FBXMeshLoader::ComputeBoundingBox(const TArray<FBX::FSkeletalMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector)
{
    if (InVertices.IsEmpty())
    {
        OutMinVector = FVector::ZeroVector; OutMaxVector = FVector::ZeroVector;
        return;
    }
    OutMinVector = InVertices[0].Position;
    OutMaxVector = InVertices[0].Position;
    for (int32 i = 1; i < InVertices.Num(); ++i)
    {
        OutMinVector = FVector::Min(OutMinVector, InVertices[i].Position);
        OutMaxVector = FVector::Max(OutMaxVector, InVertices[i].Position);
    }
}

void FBXMeshLoader::CalculateTangent(FBX::FSkeletalMeshVertex& PivotVertex, const FBX::FSkeletalMeshVertex& Vertex1, const FBX::FSkeletalMeshVertex& Vertex2) { /* TODO: Implement if needed */ }
bool FBXMeshLoader::ReconstructVertexAttributes(const FBX::MeshRawData& RawMeshData, TArray<FVector>& OutControlPointNormals, TArray<FVector>& OutPolygonVertexNormals, TArray<FVector2D>& OutControlPointUVs, TArray<FVector2D>& OutPolygonVertexUVs)
{
    int32 ControlPointCount = RawMeshData.ControlPoints.Num();
    int32 PolygonVertexCount = RawMeshData.PolygonVertexIndices.Num();

    // --- 출력 배열 초기화 및 기본값으로 채우기 ---
    OutControlPointNormals.Empty(ControlPointCount);
    OutControlPointNormals.Reserve(ControlPointCount); // 필요한 경우 용량 미리 확보
    for (int32 i = 0; i < ControlPointCount; ++i) {
        OutControlPointNormals.Add(FVector(0, 0, 1)); // 기본값
    }

    OutPolygonVertexNormals.Empty(PolygonVertexCount);
    OutPolygonVertexNormals.Reserve(PolygonVertexCount);
    for (int32 i = 0; i < PolygonVertexCount; ++i) {
        OutPolygonVertexNormals.Add(FVector(0, 0, 1)); // 기본값
    }

    OutControlPointUVs.Empty(ControlPointCount);
    OutControlPointUVs.Reserve(ControlPointCount);
    for (int32 i = 0; i < ControlPointCount; ++i) {
        OutControlPointUVs.Add(FVector2D(0, 0));    // 기본값
    }

    OutPolygonVertexUVs.Empty(PolygonVertexCount);
    OutPolygonVertexUVs.Reserve(PolygonVertexCount);
    for (int32 i = 0; i < PolygonVertexCount; ++i) {
        OutPolygonVertexUVs.Add(FVector2D(0, 0));    // 기본값
    }

    // --- 법선 데이터 처리 ---
    const FBX::MeshRawData::AttributeData& NormalData = RawMeshData.NormalData;
    if (NormalData.MappingMode != FbxLayerElement::eNone)
    {
        auto GetNormalValue = [&](int DataIndex) -> FVector {
            if (DataIndex >= 0 && NormalData.DataVec4.IsValidIndex(DataIndex)) {
                // ConvertFbxNormal은 FBX 좌표를 엔진 좌표로 변환하는 함수여야 함
                return FBX::ConvertFbxNormal(NormalData.DataVec4[DataIndex]);
            }
            return FVector(0, 0, 1); // 데이터 접근 실패 시 기본값
            };

        if (NormalData.MappingMode == FbxLayerElement::eByControlPoint)
        {
            if (NormalData.ReferenceMode == FbxLayerElement::eDirect)
            {
                // DataVec4.Num()과 ControlPointCount 중 작은 값까지만 순회 (안전성)
                int32 LoopCount = FMath::Min(ControlPointCount, NormalData.DataVec4.Num());
                for (int32 i = 0; i < LoopCount; ++i) {
                    OutControlPointNormals[i] = GetNormalValue(i);
                }

            }
            else if (NormalData.ReferenceMode == FbxLayerElement::eIndexToDirect)
            {
                // IndexArray.Num()과 ControlPointCount 중 작은 값까지만 순회
                int32 LoopCount = FMath::Min(ControlPointCount, NormalData.IndexArray.Num());
                for (int32 i = 0; i < LoopCount; ++i)
                {
                    int idx = NormalData.IndexArray[i];
                    OutControlPointNormals[i] = GetNormalValue(idx);
                }
            }
            // else: 지원하지 않는 참조 모드. 이미 기본값으로 채워져 있음.
        }
        else if (NormalData.MappingMode == FbxLayerElement::eByPolygonVertex)
        {
            if (NormalData.ReferenceMode == FbxLayerElement::eDirect)
            {
                int32 LoopCount = FMath::Min(PolygonVertexCount, NormalData.DataVec4.Num());
                for (int32 i = 0; i < LoopCount; ++i)
                {
                    OutPolygonVertexNormals[i] = GetNormalValue(i);
                }

            }
            else if (NormalData.ReferenceMode == FbxLayerElement::eIndexToDirect)
            {
                int32 LoopCount = FMath::Min(PolygonVertexCount, NormalData.IndexArray.Num());
                for (int32 i = 0; i < LoopCount; ++i) {
                    int idx = NormalData.IndexArray[i];
                    OutPolygonVertexNormals[i] = GetNormalValue(idx);
                }

            }
        }
    }

    // --- UV 데이터 처리 (법선과 유사한 방식으로) ---
    const FBX::MeshRawData::AttributeData& UVData = RawMeshData.UVData;
    if (UVData.MappingMode != FbxLayerElement::eNone)
    {
        auto GetUVValue = [&](int DataIndex) -> FVector2D {
            if (DataIndex >= 0 && UVData.DataVec2.IsValidIndex(DataIndex)) {
                return FBX::ConvertFbxUV(UVData.DataVec2[DataIndex]);
            }
            return FVector2D(0, 0); // 기본값
            };

        if (UVData.MappingMode == FbxLayerElement::eByControlPoint)
        {
            if (UVData.ReferenceMode == FbxLayerElement::eDirect)
            {
                int32 LoopCount = FMath::Min(ControlPointCount, UVData.DataVec2.Num());
                for (int32 i = 0; i < LoopCount; ++i)
                    OutControlPointUVs[i] = GetUVValue(i);
                if (LoopCount < ControlPointCount) { /* 로그 */ }
            }
            else if (UVData.ReferenceMode == FbxLayerElement::eIndexToDirect)
            {
                int32 LoopCount = FMath::Min(ControlPointCount, UVData.IndexArray.Num());
                for (int32 i = 0; i < LoopCount; ++i) {
                    int idx = UVData.IndexArray[i];
                    OutControlPointUVs[i] = GetUVValue(idx);
                }
                if (LoopCount < ControlPointCount) { /* 로그 */ }
            }
        }
        else if (UVData.MappingMode == FbxLayerElement::eByPolygonVertex)
        {
            if (UVData.ReferenceMode == FbxLayerElement::eDirect)
            {
                int32 LoopCount = FMath::Min(PolygonVertexCount, UVData.DataVec2.Num());
                for (int32 i = 0; i < LoopCount; ++i)
                    OutPolygonVertexUVs[i] = GetUVValue(i);
                if (LoopCount < PolygonVertexCount) { /* 로그 */ }
            }
            else if (UVData.ReferenceMode == FbxLayerElement::eIndexToDirect)
            {
                int32 LoopCount = FMath::Min(PolygonVertexCount, UVData.IndexArray.Num());
                for (int32 i = 0; i < LoopCount; ++i) {
                    int idx = UVData.IndexArray[i];
                    OutPolygonVertexUVs[i] = GetUVValue(idx);
                }
                if (LoopCount < PolygonVertexCount) { /* 로그 */ }
            }
        }
    }

    return true; // 항상 성공 (데이터 없으면 기본값 사용)
}
bool FBXMeshLoader::CreateTextureFromFile(const FWString& Filename)
{
    if (FEngineLoop::ResourceManager.GetTexture(Filename))
    {
        return true;
    }

    HRESULT hr = FEngineLoop::ResourceManager.LoadTextureFromFile(FEngineLoop::GraphicDevice.Device, nullptr, Filename.c_str());

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}
bool FBXMeshLoader::CreateMaterialSubsetsInternal(const FBX::MeshRawData& RawMeshData, const TMap<FName, int32>& MaterialNameToIndexMap, FBX::FSkeletalMeshRenderData& OutSkeletalMesh)
{
    OutSkeletalMesh.Subsets.Empty();
    if (OutSkeletalMesh.Indices.IsEmpty()) return true; // 인덱스가 없으면 서브셋 필요 없음 (성공)
    if (OutSkeletalMesh.Materials.IsEmpty()) return false; // 재질 없이는 서브셋 생성 불가 (실패)

    const int32 PolygonCount = RawMeshData.PolygonVertexIndices.Num() / 3;
    const FBX::MeshRawData::MaterialMappingInfo& MappingInfo = RawMeshData.MaterialMapping;

    bool bSuccess = true; // 처리 성공 여부 플래그
    bool bHandled = false; // 서브셋 생성이 처리되었는지 여부

    // --- Case 1: eByPolygon 모드이고 인덱스 배열이 유효한 경우 ---
    if (MappingInfo.MappingMode == FbxLayerElement::eByPolygon && MappingInfo.IndexArray.IsValidIndex(PolygonCount - 1))
    {
        int32 CurrentMaterialLocalIndex = -1;
        uint32 CurrentSubsetStartIndex = 0;
        uint32 IndicesProcessed = 0;

        for (int PolyIndex = 0; PolyIndex < PolygonCount; ++PolyIndex)
        {
            int32 MaterialSlotIndex = MappingInfo.IndexArray[PolyIndex];
            FName MaterialName = RawMeshData.MaterialNames.IsValidIndex(MaterialSlotIndex) ? RawMeshData.MaterialNames[MaterialSlotIndex] : NAME_None;
            int32 MaterialLocalIndex = 0;
            const int32* FoundIndexPtr = MaterialNameToIndexMap.Find(MaterialName);
            if (FoundIndexPtr) MaterialLocalIndex = *FoundIndexPtr;
            else MaterialLocalIndex = 0; // 기본값 0 사용

            if (PolyIndex == 0)
            {
                CurrentMaterialLocalIndex = MaterialLocalIndex;
                CurrentSubsetStartIndex = 0;
            }
            else if (MaterialLocalIndex != CurrentMaterialLocalIndex)
            {
                FBX::FMeshSubset Subset;
                Subset.MaterialIndex = CurrentMaterialLocalIndex;
                Subset.IndexStart = CurrentSubsetStartIndex;
                Subset.IndexCount = IndicesProcessed - CurrentSubsetStartIndex;
                if (Subset.IndexCount > 0) OutSkeletalMesh.Subsets.Add(Subset);
                CurrentMaterialLocalIndex = MaterialLocalIndex;
                CurrentSubsetStartIndex = IndicesProcessed;
            }
            IndicesProcessed += 3;
            if (PolyIndex == PolygonCount - 1)
            {
                FBX::FMeshSubset Subset;
                Subset.MaterialIndex = CurrentMaterialLocalIndex;
                Subset.IndexStart = CurrentSubsetStartIndex;
                Subset.IndexCount = IndicesProcessed - CurrentSubsetStartIndex;
                if (Subset.IndexCount > 0) OutSkeletalMesh.Subsets.Add(Subset);
            }
        }
        bHandled = true; // eByPolygon 처리 완료
    }

    // --- Case 2: eAllSame 모드 또는 eByPolygon 실패 또는 다른 모드 ---
    // bHandled 플래그를 사용하여 위에서 처리되지 않은 경우 이 로직 실행
    if (!bHandled)
    {
        // eAllSame 로직 또는 eByPolygon 실패 시의 대체 로직
        FBX::FMeshSubset Subset;
        FName MaterialName = !RawMeshData.MaterialNames.IsEmpty() ? RawMeshData.MaterialNames[0] : NAME_None;
        int32 MaterialLocalIndex = 0;
        const int32* FoundIndexPtr = MaterialNameToIndexMap.Find(MaterialName);
        if (FoundIndexPtr) MaterialLocalIndex = *FoundIndexPtr;
        else MaterialLocalIndex = 0; // 기본값 0 사용

        Subset.MaterialIndex = MaterialLocalIndex;
        Subset.IndexStart = 0;
        Subset.IndexCount = OutSkeletalMesh.Indices.Num();
        if (Subset.IndexCount > 0)
        {
            OutSkeletalMesh.Subsets.Add(Subset);
        }
        else if (!OutSkeletalMesh.Indices.IsEmpty())
        {
            // 인덱스는 있는데 서브셋 카운트가 0인 경우?
            bSuccess = false;
        }
        bHandled = true; // 처리 완료
    }

    // --- 최종 유효성 검사 ---
    uint32 TotalSubsetIndices = 0;
    for (const auto& sub : OutSkeletalMesh.Subsets)
    {
        TotalSubsetIndices += sub.IndexCount;
    }
    if (TotalSubsetIndices != OutSkeletalMesh.Indices.Num())
    {
        // 서브셋 인덱스 합계가 전체 인덱스 수와 다르면 문제 발생
        bSuccess = false;
        OutSkeletalMesh.Subsets.Empty(); // 잘못된 서브셋 정보 제거
        // 오류 발생 시 단일 기본 서브셋 생성 시도 (선택 사항)
        if (!OutSkeletalMesh.Indices.IsEmpty() && !OutSkeletalMesh.Materials.IsEmpty())
        {
            FBX::FMeshSubset DefaultSubset;
            DefaultSubset.MaterialIndex = 0;
            DefaultSubset.IndexStart = 0;
            DefaultSubset.IndexCount = OutSkeletalMesh.Indices.Num();
            OutSkeletalMesh.Subsets.Add(DefaultSubset);
            bSuccess = true; // 기본 서브셋 생성 성공으로 간주
        }
    }

    return bSuccess;

}
void FBXMeshLoader::ExtractAttributeRaw(FbxLayerElementTemplate<FbxVector4>* ElementVec4, FbxLayerElementTemplate<FbxVector2>* ElementVec2, FBX::MeshRawData::AttributeData& AttrData)
{
    // 먼저 유효한 Element 포인터가 있는지 확인
    FbxLayerElement* BaseElement = ElementVec4 ? static_cast<FbxLayerElement*>(ElementVec4) : static_cast<FbxLayerElement*>(ElementVec2);
    if (!BaseElement) {
        AttrData.MappingMode = FbxLayerElement::eNone; // 요소 없음을 표시
        return;
    }

    // MappingMode와 ReferenceMode는 기본 클래스에서 가져올 수 있음
    AttrData.MappingMode = BaseElement->GetMappingMode();
    AttrData.ReferenceMode = BaseElement->GetReferenceMode();

    // GetDirectArray와 GetIndexArray는 파생 클래스 포인터에서 호출해야 함
    if (ElementVec4) // FbxVector4 타입 (예: Normal)
    {
        const auto& DirectArray = ElementVec4->GetDirectArray();
        const auto& IndexArray = ElementVec4->GetIndexArray();
        int DataCount = DirectArray.GetCount();
        int IdxCount = IndexArray.GetCount();

        AttrData.DataVec4.Reserve(DataCount);
        for (int i = 0; i < DataCount; ++i) AttrData.DataVec4.Add(DirectArray.GetAt(i));

        if (AttrData.ReferenceMode == FbxLayerElement::eIndexToDirect || AttrData.ReferenceMode == FbxLayerElement::eIndex) {
            AttrData.IndexArray.Reserve(IdxCount);
            for (int i = 0; i < IdxCount; ++i)
                AttrData.IndexArray.Add(IndexArray.GetAt(i));
        }
    }
    else if (ElementVec2) // FbxVector2 타입 (예: UV)
    {
        const auto& DirectArray = ElementVec2->GetDirectArray();
        const auto& IndexArray = ElementVec2->GetIndexArray();
        int DataCount = DirectArray.GetCount();
        int IdxCount = IndexArray.GetCount();

        AttrData.DataVec2.Reserve(DataCount);
        for (int i = 0; i < DataCount; ++i) AttrData.DataVec2.Add(DirectArray.GetAt(i));

        if (AttrData.ReferenceMode == FbxLayerElement::eIndexToDirect || AttrData.ReferenceMode == FbxLayerElement::eIndex) {
            AttrData.IndexArray.Reserve(IdxCount);
            for (int i = 0; i < IdxCount; ++i)
                AttrData.IndexArray.Add(IndexArray.GetAt(i));
        }
    }
    // else: 다른 타입의 Element가 있다면 여기에 추가 (예: FbxColor)
}

bool FBXMeshLoader::ExtractSingleMeshRawData(FbxNode* Node, FBX::MeshRawData& OutRawData, const TMap<FbxSurfaceMaterial*, FName>& MaterialPtrToNameMap)
{
    FbxMesh* Mesh = Node->GetMesh();
    if (!Mesh) return false;
    OutRawData.NodeName = FName(Node->GetName());

    ExtractAttributeRaw(Mesh->GetElementNormal(0), nullptr, OutRawData.NormalData);
    ExtractAttributeRaw(nullptr, Mesh->GetElementUV(0), OutRawData.UVData);

    int32 ControlPointCount = Mesh->GetControlPointsCount();

    if (ControlPointCount <= 0) return false;

    OutRawData.ControlPoints.Reserve(ControlPointCount);

    FbxVector4* FbxControlPoints = Mesh->GetControlPoints();
    for (int32 i = 0; i < ControlPointCount; ++i) OutRawData.ControlPoints.Add(FBX::ConvertFbxPosition(FbxControlPoints[i]));

    int32 PolygonVertexCount = Mesh->GetPolygonVertexCount();
    int32 PolygonCount = Mesh->GetPolygonCount();

    if (PolygonVertexCount <= 0 || PolygonCount <= 0 || PolygonVertexCount != PolygonCount * 3) return false;
    OutRawData.PolygonVertexIndices.Reserve(PolygonVertexCount);
    int* FbxPolygonVertices = Mesh->GetPolygonVertices();
    for (int32 i = 0; i < PolygonVertexCount; ++i) OutRawData.PolygonVertexIndices.Add(FbxPolygonVertices[i]);



    int DeformerCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int deformerIdx = 0; deformerIdx < DeformerCount; ++deformerIdx)
    {
        FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin));
        if (!Skin)
            continue;
        int ClusterCount = Skin->GetClusterCount();
        for (int clusterIdx = 0; clusterIdx < ClusterCount; ++clusterIdx)
        {
            FbxCluster* Cluster = Skin->GetCluster(clusterIdx);
            if (!Cluster)
                continue;
            FbxNode* BoneNode = Cluster->GetLink();
            if (!BoneNode)
                continue;
            FBX::MeshRawData::RawInfluence Influence;
            Influence.BoneName = FName(BoneNode->GetName());
            int InfluenceCount = Cluster->GetControlPointIndicesCount();
            if (InfluenceCount > 0)
            {
                Influence.ControlPointIndices.Reserve(InfluenceCount);
                Influence.ControlPointWeights.Reserve(InfluenceCount);
                int* Indices = Cluster->GetControlPointIndices();
                double* Weights = Cluster->GetControlPointWeights();
                for (int i = 0; i < InfluenceCount; ++i)
                {
                    Influence.ControlPointIndices.Add(Indices[i]);
                    Influence.ControlPointWeights.Add(Weights[i]);
                }
                OutRawData.SkinningInfluences.Add(std::move(Influence));
            }
        }
    }

    int MaterialCount = Node->GetMaterialCount();
    OutRawData.MaterialNames.Reserve(MaterialCount);
    for (int matIdx = 0; matIdx < MaterialCount; ++matIdx) {
        FbxSurfaceMaterial* FbxMat = Node->GetMaterial(matIdx);
        const FName* MatNamePtr = FbxMat ? MaterialPtrToNameMap.Find(FbxMat) : nullptr;
        OutRawData.MaterialNames.Add(MatNamePtr ? *MatNamePtr : NAME_None);
    }
    FbxLayerElementMaterial* MaterialElement = Mesh->GetLayer(0) ? Mesh->GetLayer(0)->GetMaterials() : nullptr;
    if (MaterialElement) {
        OutRawData.MaterialMapping.MappingMode = MaterialElement->GetMappingMode();
        if (OutRawData.MaterialMapping.MappingMode == FbxLayerElement::eByPolygon)
        {
            const auto& IndexArray = MaterialElement->GetIndexArray(); int IdxCount = IndexArray.GetCount();
            if (IdxCount == PolygonCount) {
                OutRawData.MaterialMapping.IndexArray.Reserve(IdxCount); for (int i = 0; i < IdxCount; ++i) OutRawData.MaterialMapping.IndexArray.Add(IndexArray.GetAt(i));
            }
            else { OutRawData.MaterialMapping.MappingMode = FbxLayerElement::eAllSame; }
        }
    }
    else { OutRawData.MaterialMapping.MappingMode = FbxLayerElement::eAllSame; }
    return true;
}
