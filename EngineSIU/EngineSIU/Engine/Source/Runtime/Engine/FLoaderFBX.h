#pragma once
#include "Define.h"

// --- 기본 타입 및 컨테이너 ---
#include "Math/Vector.h"      // FVector, FVector2D 포함 가정
#include "Math/Vector4.h"     // FVector4 포함 가정
#include "Math/Matrix.h"      // FMatrix 포함 가정
#include "Container/Array.h" // TArray 포함 가정
#include "Container/Map.h"    // TMap 포함 가정
#include "UObject/NameTypes.h" // FName 포함 가정
#include "Components/Mesh/SkeletalMesh.h"

#include <fbxsdk.h>


namespace std
{
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher; // T 타입에 대한 표준 해시 함수 객체
        // 기존 시드 값과 새로운 해시 값을 조합 (Boost::hash_combine 방식)
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<> // 템플릿 특수화 선언
    struct hash<FBX::FMeshVertex> // std::hash를 FBX::FMeshVertex 타입에 대해 특수화
    {
        // 함수 호출 연산자 오버로딩: 해시 계산 수행
        size_t operator()(const FBX::FMeshVertex& Key) const noexcept
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

namespace FBX { struct FSkeletalMeshRenderData; }

// --- FBX 로더 클래스 ---
class FLoaderFBX
{
public:
    ~FLoaderFBX();

    static USkeletalMesh* CreateSkeletalMesh(const FString& Path);
    // FBX 파일 로드
    bool LoadFBXFile(const FString& Filepath, ID3D11Device* Device);

    // 스키닝 업데이트 및 적용 (CPU 스키닝)
    bool UpdateAndApplySkinning(ID3D11DeviceContext* DeviceContext);
    // 렌더링
    void Render(ID3D11DeviceContext* DeviceContext);
    FBX::FSkeletalMeshRenderData* LoadSkeletalMeshAsset(const FString& PathFileName, ID3D11Device* Device);
    FBX::FSkeletalMeshRenderData* GetSkeletalMesh(const FString& PathFileName);
    void ClearAll();

    // --- 데이터 접근자 (필요시 추가) ---
    const TArray<FBX::FMeshVertex>& GetVertices() const { return BindPoseVertices; }
    const TArray<uint32>& GetIndices() const { return FinalIndices; }
    const TArray<FBX::FBoneInfo>& GetBones() const { return Bones; }
    int32 GetBoneIndex(const FName& BoneName) const;
    const FBX::FBoneInfo* GetBoneInfo(int32 BoneIndex) const;

    void UpdateWorldTransforms();


public:
    TArray<FName> GetBoneNames() const;
    bool SetBoneLocalMatrix(uint32_t BoneIndex, const FMatrix& NewLocalMatrix);
    FMatrix GetBoneLocalMatrix(uint32_t BoneIndex) const;
    bool SetBoneWorldMatrix(uint32_t BoneIndex, const FMatrix& NewWorldMatrix);
    FMatrix GetBoneWorldMatrix(uint32_t BoneIndex) const;
    uint32_t GetBoneIndexByName(const FName& BoneName) const;
    FBX::FFbxMaterialInfo  ProcessMaterial(FbxSurfaceMaterial* FbxMaterial);
private:
    void CalculateInitialLocalTransforms();

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
    FWString ProcessTexturePath(FbxFileTexture* Texture);
    // --- 멤버 변수 ---
    // FBX SDK
    FbxManager* SdkManager = nullptr;
    FbxScene* Scene = nullptr;

    ID3D11Buffer* DynamicVertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;
  
    TArray<FBX::FFbxMaterialInfo> Materials;
    TMap<FbxSurfaceMaterial*, int32> MaterialToIndexMap;
    FString LoadedFBXFilePath;
    FWString LoadedFBXFileDirectory;
public:
    TArray<FBX::FMeshVertex> BindPoseVertices; // 바인드 포즈 상태의 최종 정점 데이터
    TArray<uint32> FinalIndices;          // 최종 인덱스 데이터
    TArray<FBX::FBoneInfo> Bones;              // 스켈레톤 뼈 정보
    TMap<FName, int32> BoneNameToIndexMap; // 뼈 이름 -> 인덱스 맵
};
