#pragma once
#include "Define.h"

// --- 기본 타입 및 컨테이너 ---
#include "Math/Vector.h"      // FVector, FVector2D 포함 가정
#include "Math/Vector4.h"     // FVector4 포함 가정
#include "Math/Matrix.h"      // FMatrix 포함 가정
#include "Container/Array.h" // TArray 포함 가정
#include "Container/Map.h"    // TMap 포함 가정
#include "UObject/NameTypes.h" // FName 포함 가정

#include <fbxsdk.h>

#ifndef MAX_BONE_INFLUENCES
#define MAX_BONE_INFLUENCES 4
#endif


// --- 정점 구조체 ---
struct FMeshVertex
{
    FVector Position;
    FVector Normal;
    FVector2D TexCoord;
    uint32 BoneIndices[MAX_BONE_INFLUENCES] = { 0 };
    float BoneWeights[MAX_BONE_INFLUENCES] = { 0.0f };

    // TMap에서 키로 사용하기 위한 비교 연산자
    bool operator==(const FMeshVertex& Other) const
    {
        // 모든 멤버 비교 (부동 소수점 비교 시 허용 오차 고려 필요할 수 있음)
        if (Position != Other.Position ||
            Normal != Other.Normal ||
            TexCoord != Other.TexCoord)
        {
            return false;
        }
        for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
        {
            if (BoneIndices[i] != Other.BoneIndices[i] ||
                !FMath::IsNearlyEqual(BoneWeights[i], Other.BoneWeights[i])) // FMath::IsNearlyEqual 사용 권장
            {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const FMeshVertex& Other) const
    {
        return !(*this == Other);
    }
};
namespace std // 표준 라이브러리 네임스페이스 내에 정의해야 함
{
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher; // T 타입에 대한 표준 해시 함수 객체
        // 기존 시드 값과 새로운 해시 값을 조합 (Boost::hash_combine 방식)
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<> // 템플릿 특수화 선언
    struct hash<FMeshVertex> // std::hash를 FMeshVertex 타입에 대해 특수화
    {
        // 함수 호출 연산자 오버로딩: 해시 계산 수행
        size_t operator()(const FMeshVertex& Key) const noexcept
        {
            size_t seed = 0; // 해시 시드 초기화

           
            // Position (FVector)
            hash_combine(seed, std::hash<float>()(Key.Position.X));
            hash_combine(seed, std::hash<float>()(Key.Position.Y));
            hash_combine(seed, std::hash<float>()(Key.Position.Z));

            // Normal (FVector)
            hash_combine(seed, std::hash<float>()(Key.Normal.X));
            hash_combine(seed, std::hash<float>()(Key.Normal.Y));
            hash_combine(seed, std::hash<float>()(Key.Normal.Z));

            // TexCoord (FVector2D)
            hash_combine(seed, std::hash<float>()(Key.TexCoord.X));
            hash_combine(seed, std::hash<float>()(Key.TexCoord.Y));

            // BoneIndices (array of uint32)
            std::hash<uint32> uint_hasher; // 루프 밖에서 생성하여 효율성 증대
            for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
            {
                hash_combine(seed, uint_hasher(Key.BoneIndices[i]));
            }

            // BoneWeights (array of float)
            std::hash<float> float_hasher; // 루프 밖에서 생성
            for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
            {
                hash_combine(seed, float_hasher(Key.BoneWeights[i]));
            }

            return seed; // 최종 조합된 해시 값 반환
        }
    };
}
struct FBoneInfo
{
    FName Name;
    int32 ParentIndex = INDEX_NONE; // 부모 뼈의 인덱스 (루트면 INDEX_NONE)
    FMatrix InverseBindPoseMatrix = FMatrix::Identity; // 역 바인드 포즈 행렬
    FMatrix BindPoseMatrix = FMatrix::Identity; // 역 바인드 포즈 행렬
    // FMatrix CurrentTransform; // 현재 프레임의 최종 변환 행렬 (외부에서 계산하여 전달)
};

// --- FBX 로더 클래스 ---
class FLoaderFBX
{
public:
    ~FLoaderFBX();

    // FBX 파일 로드
    bool LoadFBXFile(const FString& Filepath, ID3D11Device* Device);

    // 스키닝 업데이트 및 적용 (CPU 스키닝)
    bool UpdateAndApplySkinning(ID3D11DeviceContext* DeviceContext, const TArray<FMatrix>& FinalBoneTransforms);

    // 렌더링
    void Render(ID3D11DeviceContext* DeviceContext);

    // --- 데이터 접근자 (필요시 추가) ---
    const TArray<FMeshVertex>& GetVertices() const { return BindPoseVertices; }
    const TArray<uint32>& GetIndices() const { return FinalIndices; }
    const TArray<FBoneInfo>& GetBones() const { return Bones; }
    int32 GetBoneIndex(const FName& BoneName) const;
    const FBoneInfo* GetBoneInfo(int32 BoneIndex) const;

private:
    // --- 초기화 및 해제 ---
    bool InitializeFBXSDK();
    void ShutdownFBXSDK();
    bool CreateBuffers(ID3D11Device* Device);
    void ReleaseBuffers();

    // --- FBX 처리 ---
    bool ProcessScene();
    void ProcessNodeRecursive(FbxNode* Node);
    bool ProcessMesh(FbxMesh* Mesh);
    bool ProcessSkeletonHierarchy(FbxNode* RootNode);
    void ProcessSkeletonNodeRecursive(FbxNode* Node, int32 CurrentParentBoneIndex);
    void ExtractSkinningData(FbxMesh* Mesh, TArray<struct FControlPointSkinningData>& OutCpSkinData);
    void FinalizeVertexData(
        const TArray<FVector>& ControlPointPositions,
        const TArray<int32>& PolygonIndices,
        const TArray<FVector>& ControlPointNormals,
        const TArray<FVector>& PolygonVertexNormals,
        const FbxGeometryElementNormal* NormalElement,
        const TArray<FVector2D>& ControlPointUVs,
        const TArray<FVector2D>& PolygonVertexUVs,
        const FbxGeometryElementUV* UVElement,
        const TArray<struct FControlPointSkinningData>& CpSkinData
    );

    // --- 데이터 추출 헬퍼 ---
    template<typename TElementType, typename TDataType> // TDataType: FVector or FVector2D
    bool GetElementData(
        FbxMesh* Mesh,
        TElementType* Element,
        int32 ControlPointCount,
        int32 PolygonVertexCount,
        const TArray<int32>& PolygonIndices,
        TArray<TDataType>& OutControlPointData,
        TArray<TDataType>& OutPolygonVertexData);

    // --- 좌표계 변환 헬퍼 ---
    FVector ConvertFbxPosition(const FbxVector4& Vector);
    FVector ConvertFbxNormal(const FbxVector4& Vector);
    FVector2D ConvertFbxUV(const FbxVector2& Vector);
    // FMatrix::ConvertFbxAMatrixToFMatrix 사용

    // --- 멤버 변수 ---
    // FBX SDK
    FbxManager* SdkManager = nullptr;
    FbxScene* Scene = nullptr;

    ID3D11Buffer* DynamicVertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;
  
    // 로드된 데이터
public:
    TArray<FMeshVertex> BindPoseVertices; // 바인드 포즈 상태의 최종 정점 데이터
    TArray<uint32> FinalIndices;          // 최종 인덱스 데이터
    TArray<FBoneInfo> Bones;              // 스켈레톤 뼈 정보
    TMap<FName, int32> BoneNameToIndexMap; // 뼈 이름 -> 인덱스 맵
};
