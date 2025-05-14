#pragma once

#include "Define.h"
#include "FBXStructs.h"

class USkeleton;

namespace FBX
{
    struct MeshRawData;
    struct FBXInfo;
}

/// Skeletal 메시 데이터를 생성하는 메시 로더
class FBXMeshLoader
{
public:
    /// 파싱된 Raw 메시 데이터를 기반으로 렌더링 가능한 스켈레탈 메시 데이터를 생성
    static bool ConvertToSkeletalMesh(
        const TArray<FBX::MeshRawData>& RawMeshData,
        const FBX::FBXInfo& FullFBXInfo,
        FBX::FSkeletalMeshRenderData& OutSkeletalMesh,
        USkeleton* OutSkeleton
    );

    /// 메시 정점으로부터 AABB를 계산
    static void ComputeBoundingBox(
        const TArray<FBX::FSkeletalMeshVertex>& InVertices,
        FVector& OutMinVector,
        FVector& OutMaxVector
    );
    static void ExtractAttributeRaw(FbxLayerElementTemplate<FbxVector4>* ElementVec4, FbxLayerElementTemplate<FbxVector2>* ElementVec2, FBX::MeshRawData::AttributeData& AttrData);
    static bool ExtractSingleMeshRawData(FbxNode* Node, FBX::MeshRawData& OutRawData, const TMap<FbxSurfaceMaterial*, FName>& MaterialPtrToNameMap);
    static bool CreateTextureFromFile(const FWString& Filename);
    static bool CreateMaterialSubsetsInternal(const FBX::MeshRawData& RawMeshData, const TMap<FName, int32>& MaterialNameToIndexMap, FBX::FSkeletalMeshRenderData& OutSkeletalMesh);

private:
    /// 3개의 정점을 기반으로 탄젠트를 계산 (현재 미사용 시에도 정의는 유지)
    static void CalculateTangent(
        FBX::FSkeletalMeshVertex& PivotVertex,
        const FBX::FSkeletalMeshVertex& Vertex1,
        const FBX::FSkeletalMeshVertex& Vertex2
    );
    static bool ReconstructVertexAttributes(
        const FBX::MeshRawData& RawMeshData,
        TArray<FVector>& OutControlPointNormals,
        TArray<FVector>& OutPolygonVertexNormals,
        TArray<FVector2D>& OutControlPointUVs,
        TArray<FVector2D>& OutPolygonVertexUVs
    );

    struct FControlPointSkinningData
    {
        struct FBoneInfluence
        {
            int32 BoneIndex = INDEX_NONE;
            float Weight = 0.0f;
            bool operator>(const FBoneInfluence& Other) const { return Weight > Other.Weight; }
        };
        TArray<FBoneInfluence> Influences;
        void NormalizeWeights(int32 MaxInfluences)
        {
            if (Influences.IsEmpty()) return; // 1. 영향이 없으면 종료

            // 2. 모든 가중치의 합 계산
            float TotalWeight = 0.0f;
            for (int32 i = 0; i < Influences.Num(); ++i) TotalWeight += Influences[i].Weight;

            // 3. 가중치 합으로 각 가중치 정규화 (합이 1이 되도록)
            if (TotalWeight > KINDA_SMALL_NUMBER)
            {
                for (int32 i = 0; i < Influences.Num(); ++i) Influences[i].Weight /= TotalWeight;
            }
            else if (!Influences.IsEmpty())
            {

                float EqualWeight = 1.0f / Influences.Num(); // 균등 분배
                for (int32 i = 0; i < Influences.Num(); ++i) Influences[i].Weight = EqualWeight;
            }

            // 4. 가중치 크기 순으로 정렬 (내림차순)
            Influences.Sort([](const FBoneInfluence& Lhs, const FBoneInfluence& Rhs) {
                return Lhs.Weight > Rhs.Weight;
                });

            // 5. 최대 영향 본 개수(MaxInfluences) 제한
            if (Influences.Num() > MaxInfluences)
            {
                Influences.SetNum(MaxInfluences);

                if (MaxInfluences > 0)
                {
                    Influences.RemoveAt(Influences.Num() - MaxInfluences);
                }
                else
                {
                    Influences.Empty(); // MaxInfluences가 0이면 모든 영향 제거
                }


                // 최대 개수 제한 후 다시 정규화 (제거된 본들의 가중치를 나머지 본들에 재분배)
                TotalWeight = 0.0f;
                for (int32 i = 0; i < Influences.Num(); ++i) TotalWeight += Influences[i].Weight;

                if (TotalWeight > KINDA_SMALL_NUMBER) {
                    for (int32 i = 0; i < Influences.Num(); ++i) Influences[i].Weight /= TotalWeight;
                }
                else if (!Influences.IsEmpty()) { // 모든 가중치가 0이 된 경우 (거의 발생 안 함)
                    Influences[0].Weight = 1.0f; // 첫 번째 본에 모든 가중치 할당
                    for (int32 i = 1; i < Influences.Num(); ++i) Influences[i].Weight = 0.0f;
                }
            }

            // 6. 최종 정규화 (부동 소수점 오류로 합이 정확히 1이 아닐 수 있으므로, 마지막 가중치 조정)
            if (!Influences.IsEmpty())
            {
                float CurrentSum = 0.0f;
                // 마지막 요소를 제외한 모든 요소의 가중치 합 계산
                for (int32 i = 0; i < Influences.Num() - 1; ++i) CurrentSum += Influences[i].Weight;

                int32 LastIndex = Influences.Num() - 1;
                if (LastIndex >= 0)
                {
                    Influences[LastIndex].Weight = FMath::Max(0.0f, 1.0f - CurrentSum);
                }
            }
        }
    };
};
