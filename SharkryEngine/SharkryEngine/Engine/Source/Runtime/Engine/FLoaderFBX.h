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

#include <functional>
#include <unordered_map> 
#include "FSkeletalMeshDebugger.h" // FSkeletalMeshDebugger 클래스 선언을 사용하기 위해 포함


class UAnimationAsset;
class USkeletalMeshComponent;
class UAnimDataModel;
enum class ETransformChannel;

// --- FBX 로딩 관련 네임스페이스 ---
namespace FBX
{
    struct FBoneHierarchyNode;
    // --- 데이터 구조체 정의 ---
    // 재질 정보 구조체
    struct FFbxMaterialInfo
    {
        FName MaterialName = NAME_None;
        FString UniqueID; // 필요 시 사용 (FBX Material Unique ID)

        FLinearColor BaseColorFactor = FLinearColor::White;
        FLinearColor EmissiveFactor = FLinearColor::Black;
        FVector SpecularFactor = FVector(1.0f, 1.0f, 1.0f); // 전통적 Specular 값
        float MetallicFactor = 0.0f;
        float RoughnessFactor = 0.8f;
        float SpecularPower = 32.0f; // 전통적 Shininess
        float OpacityFactor = 1.0f;
        // float IOR = 1.5f; // 필요 시 추가

        FWString BaseColorTexturePath;
        FWString NormalTexturePath;
        FWString MetallicTexturePath;
        FWString RoughnessTexturePath;
        FWString SpecularTexturePath;
        FWString EmissiveTexturePath;
        FWString AmbientOcclusionTexturePath;
        FWString OpacityTexturePath;

        bool bHasBaseColorTexture = false;
        bool bHasNormalTexture = false;
        bool bHasMetallicTexture = false;
        bool bHasRoughnessTexture = false;
        bool bHasSpecularTexture = false;
        bool bHasEmissiveTexture = false;
        bool bHasAmbientOcclusionTexture = false;
        bool bHasOpacityTexture = false;

        bool bIsTransparent = false;
        bool bUsePBRWorkflow = true; // 기본적으로 PBR 선호

        FFbxMaterialInfo() = default;
    };

    // 스켈레탈 메시 정점 구조체
    struct FSkeletalMeshVertex
    {
        FVector Position;
        float R, G, B, A; // Color
        FVector Normal;
        float TangentX, TangentY, TangentZ; 
        FVector2D TexCoord;
        uint32 MaterialIndex;


        // TODO: FVector Tangent; // 탄젠트 계산 활성화 시 주석 해제 및 CalculateTangent 구현 수정 필요
        uint32 BoneIndices[MAX_BONE_INFLUENCES] = { 0 };
        float BoneWeights[MAX_BONE_INFLUENCES] = { 0.0f };

        // 비교 연산자 (정점 중복 제거용)
        bool operator==(const FSkeletalMeshVertex& Other) const
        {
            if (Position != Other.Position ||
                Normal != Other.Normal ||
                TexCoord != Other.TexCoord) // || Tangent != Other.Tangent) // 탄젠트 추가 시 비교
            {
                return false;
            }
            for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
            {
                // 가중치는 부동소수점 비교 주의
                if (BoneIndices[i] != Other.BoneIndices[i] || !FMath::IsNearlyEqual(BoneWeights[i], Other.BoneWeights[i]))
                {
                    return false;
                }
            }
            return true;
        }
        bool operator!=(const FSkeletalMeshVertex& Other) const
        {
            return !(*this == Other);
        }
    };

    // 재질 서브셋 구조체 (FMaterialSubset과 유사하게 정의)
    struct FMeshSubset
    {
        uint32 IndexStart = 0;    // 이 서브셋에 해당하는 인덱스 버퍼 시작 위치
        uint32 IndexCount = 0;    // 이 서브셋에 해당하는 인덱스 개수
        uint32 MaterialIndex = 0; // FSkeletalMeshRenderData::Materials 배열 내 인덱스
        // FString MaterialName; // 필요 시 추가 (디버깅용)
    };
    struct FSkeletalMeshInstanceRenderData
    {
        // 각 USkinnedMeshComponent 전용 (CPU 스키닝용)
        ID3D11Buffer* DynamicVertexBuffer_CPU = nullptr;
        bool bUseGpuSkinning = true; // 렌더링 시 분기 결정용
        ~FSkeletalMeshInstanceRenderData() { ReleaseBuffers(); }
        void ReleaseBuffers()
        {
            if (DynamicVertexBuffer_CPU) { DynamicVertexBuffer_CPU->Release(); DynamicVertexBuffer_CPU = nullptr; }
        }
    };
    // 최종 렌더링 데이터 구조체
    struct FSkeletalMeshRenderData
    {
        FString MeshName;
        FString FilePath;

        TArray<FSkeletalMeshVertex> BindPoseVertices; // 최종 고유 정점 배열 (바인드 포즈)
        TArray<uint32> Indices;                       // 정점 인덱스 배열

        TArray<FFbxMaterialInfo> Materials;           // 이 메시에 사용된 재질 정보 배열
        TArray<FMeshSubset> Subsets;                  // 재질별 인덱스 범위 정보

        // GPU 스키닝용 본 행렬 (BindPose^-1 * Pose)
        //TArray<FMatrix> SkinningMatrices;

        // DirectX 버퍼 포인터 (생성 후 채워짐)
        ID3D11Buffer* SharedVertexBuffer; // 불변
        ID3D11Buffer* IndexBuffer = nullptr;

        FBoundingBox Bounds;                          // 메시의 AABB

        FSkeletalMeshRenderData() = default;
        ~FSkeletalMeshRenderData() { ReleaseBuffers(); }

        FSkeletalMeshRenderData(const FSkeletalMeshRenderData&) = delete;
        FSkeletalMeshRenderData& operator=(const FSkeletalMeshRenderData&) = delete;

        FSkeletalMeshRenderData(FSkeletalMeshRenderData&& Other) noexcept
            : MeshName(std::move(Other.MeshName)), // std::move 사용
            FilePath(std::move(Other.FilePath)),
            BindPoseVertices(std::move(Other.BindPoseVertices)),
            Indices(std::move(Other.Indices)),
            Materials(std::move(Other.Materials)),
            Subsets(std::move(Other.Subsets)), // Subsets 이동 추가
            //SkinningMatrices(std::move(Other.SkinningMatrices)),
            SharedVertexBuffer(Other.SharedVertexBuffer),
            IndexBuffer(Other.IndexBuffer),
            Bounds(Other.Bounds)
        {
            Other.SharedVertexBuffer = nullptr;
            Other.IndexBuffer = nullptr;
        }

        FSkeletalMeshRenderData& operator=(FSkeletalMeshRenderData&& Other) noexcept
        {
            if (this != &Other)
            {
                ReleaseBuffers();
                MeshName = std::move(Other.MeshName);
                FilePath = std::move(Other.FilePath);
                BindPoseVertices = std::move(Other.BindPoseVertices);
                Indices = std::move(Other.Indices);
                Materials = std::move(Other.Materials);
                Subsets = std::move(Other.Subsets); // Subsets 이동 추가
                //SkinningMatrices = std::move(Other.SkinningMatrices);
                SharedVertexBuffer = Other.SharedVertexBuffer;
                IndexBuffer = Other.IndexBuffer;
                Bounds = Other.Bounds;
                Other.SharedVertexBuffer = nullptr;
                Other.IndexBuffer = nullptr;
            }
            return *this;
        }

        void ReleaseBuffers()
        {
            if (SharedVertexBuffer) { SharedVertexBuffer->Release(); SharedVertexBuffer = nullptr; }
            if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = nullptr; }
        }

        void CalculateBounds()
        {
            if (BindPoseVertices.IsEmpty())
            {
                Bounds.min = FVector::ZeroVector;
                Bounds.max = FVector::ZeroVector;
                return;
            }
            Bounds.min = BindPoseVertices[0].Position;
            Bounds.max = BindPoseVertices[0].Position;
            for (int32 i = 1; i < BindPoseVertices.Num(); ++i)
            {
                Bounds.min = FVector::Min(Bounds.min, BindPoseVertices[i].Position);
                Bounds.max = FVector::Max(Bounds.max, BindPoseVertices[i].Position);
            }
        }
    };

    // --- 중간 데이터 구조체 (파싱 결과 임시 저장용) ---
    // 이 구조체들은 FLoaderFBX 내부 구현 세부사항이므로 헤더에 노출할 필요는 없지만,
    // FLoaderFBX의 static 메서드 시그니처에서 사용되므로 여기에 선언합니다.
    struct FBXInfo;
    struct MeshRawData;
} // namespace FBX


namespace std
{
    // hash_combine 함수 템플릿 정의 (헤더에 위치)
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // std::hash<FSkeletalMeshVertex> 특수화 *선언* (구현은 cpp 파일에)
    template <>
    struct hash<FBX::FSkeletalMeshVertex>
    {
        // 함수 선언만 남기고 세미콜론으로 끝냅니다.
        size_t operator()(const FBX::FSkeletalMeshVertex& Key) const noexcept;
    };

}

namespace FBX { struct FSkeletalMeshRenderData; }

struct FLoaderFBX
{
    static bool ParseFBX(const FString& FBXFilePath, FBX::FBXInfo& OutFBXInfo);

    // Convert the Raw data to Cooked data (FSkeletalMeshRenderData)
    static bool ConvertToSkeletalMesh(const TArray<FBX::MeshRawData>& RawMeshData, const FBX::FBXInfo& FullFBXInfo, FBX::FSkeletalMeshRenderData& OutSkeletalMesh, USkeleton* OutSkeleton);
    
    static bool CreateTextureFromFile(const FWString& Filename);

    static void ComputeBoundingBox(const TArray<FBX::FSkeletalMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector);

    // TODO 아래 함수에 대한 오버로딩으로 Animation 파일만 있는 경우도 대응할것
    static void ParseFBXAnim(FbxScene* Scene, TArray<FbxNode*>& BoneNodes);

    static void ParseFBXCurveKey(FbxNode* BoneNode, FbxAnimLayer* Layer, UAnimDataModel* AnimDataModel, const FString& PropertyName, ETransformChannel TransformChannel, const char* pChannel);

    // TODO 아래 테스트 코드 지우기
    static void GenerateTestAnimationAsset();

private:
    static void CalculateTangent(FBX::FSkeletalMeshVertex& PivotVertex, const FBX::FSkeletalMeshVertex& Vertex1, const FBX::FSkeletalMeshVertex& Vertex2);
    
    // CurveKey에 사용할 Property
    static const FString TranslationChannels[3];
    static const FString RotationChannels[3];
    static const FString ScalingChannels[3];
};

struct FManagerFBX
{
public:
    static FBX::FSkeletalMeshRenderData* LoadFBXSkeletalMeshAsset(const FString& PathFileName, USkeleton* OutSkeleton);

    static void CombineMaterialIndex(FBX::FSkeletalMeshRenderData& OutFSkeletalMesh);

    static bool SaveSkeletalMeshToBinary(const FWString& FilePath, const FBX::FSkeletalMeshRenderData& SkeletalMesh);

    static bool LoadSkeletalMeshFromBinary(const FWString& FilePath, FBX::FSkeletalMeshRenderData& OutSkeletalMesh);

    static UMaterial* CreateMaterial(const FBX::FFbxMaterialInfo& materialInfo);

    static TMap<FString, UMaterial*>& GetMaterials() { return materialMap; }

    static UMaterial* GetMaterial(const FString& name);

    static int GetMaterialNum() { return materialMap.Num(); }

    static USkeletalMesh* CreateSkeletalMesh(const FString& filePath);

    static const TMap<FWString, USkeletalMesh*>& GetSkeletalMeshes();

    static USkeletalMesh* GetSkeletalMesh(const FWString& name);

    static int GetSkeletalMeshNum() { return SkeletalMeshMap.Num(); }

    static UAnimationAsset* GetAnimationAsset(const FString& name);
    static void AddAnimationAsset(const FString& name, UAnimationAsset* AnimationAsset);

private:
    inline static TMap<FString, FBX::FSkeletalMeshRenderData*> FBXSkeletalMeshMap;
    inline static TMap<FWString, USkeletalMesh*> SkeletalMeshMap;
    inline static TMap<FString, UMaterial*> materialMap;
    inline static TMap<FString, UAnimationAsset*> AnimationAssetMap;
};
