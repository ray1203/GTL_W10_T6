#pragma once
#include <cmath>
#include <algorithm>
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "Core/Container/Map.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


#define UE_LOG Console::GetInstance().AddLog

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "UserInterface/Console.h"
#include <Math/Color.h>
#include "LightDefine.h"

#define GOURAUD "LIGHTING_MODEL_GOURAUD"
#define LAMBERT "LIGHTING_MODEL_LAMBERT"
#define PHONG "LIGHTING_MODEL_BLINN_PHONG"

#ifndef MAX_BONE_INFLUENCES
#define MAX_BONE_INFLUENCES 4
#endif


struct FStaticMeshVertex
{
    float X, Y, Z;    // Position
    float R, G, B, A; // Color
    float NormalX, NormalY, NormalZ;
    float TangentX, TangentY, TangentZ;
    float U = 0, V = 0;
    uint32 MaterialIndex;
};

// Material Subset
struct FMaterialSubset
{
    uint32 IndexStart; // Index Buffer Start pos
    uint32 IndexCount; // Index Count
    uint32 MaterialIndex; // Material Index
    FString MaterialName; // Material Name
};

struct FStaticMaterial
{
    class UMaterial* Material;
    FName MaterialSlotName;
};

// OBJ File Raw Data
struct FObjInfo
{
    FWString ObjectName; // OBJ File Name. Path + FileName.obj 
    FWString FilePath; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name

    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;

    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> UVIndices;

    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

struct FObjMaterialInfo
{
    FString MaterialName;  // newmtl : Material Name.

    uint32 TextureFlag = 0;

    bool bTransparent = false; // Has alpha channel?

    FVector Diffuse = FVector(0.7f, 0.7f, 0.7f);  // Kd : Diffuse (Vector4)
    FVector Specular = FVector(0.5f, 0.5f, 0.5f);  // Ks : Specular (Vector) 
    FVector Ambient = FVector(0.01f, 0.01f, 0.01f);   // Ka : Ambient (Vector)
    FVector Emissive = FVector::ZeroVector;  // Ke : Emissive (Vector)

    float SpecularScalar = 64.f; // Ns : Specular Power (Float)
    float DensityScalar;  // Ni : Optical Density (Float)
    float TransparencyScalar; // d or Tr  : Transparency of surface (Float)
    float BumpMultiplier;     // -bm : Bump Mulitplier ex) normalMap.xy *= BumpMultiplier; 
    uint32 IlluminanceModel; // illum: illumination Model between 0 and 10. (UINT)

    /* Texture */
    FString DiffuseTextureName;  // map_Kd : Diffuse texture
    FWString DiffuseTexturePath;

    FString AmbientTextureName;  // map_Ka : Ambient texture
    FWString AmbientTexturePath;

    FString SpecularTextureName; // map_Ks : Specular texture
    FWString SpecularTexturePath;

    FString BumpTextureName;     // map_Bump : Bump texture
    FWString BumpTexturePath;

    FString AlphaTextureName;    // map_d : Alpha texture
    FWString AlphaTexturePath;
};

// Cooked Data
namespace OBJ
{
    struct FStaticMeshRenderData
    {
        FWString ObjectName;
        FString DisplayName;

        TArray<FStaticMeshVertex> Vertices;
        TArray<UINT> Indices;

        ID3D11Buffer* VertexBuffer;
        ID3D11Buffer* IndexBuffer;

        TArray<FObjMaterialInfo> Materials;
        TArray<FMaterialSubset> MaterialSubsets;

        FVector BoundingBoxMin;
        FVector BoundingBoxMax;
    };
}
struct FVertexTexture
{
    float x, y, z;    // Position
    float u, v; // Texture
};

struct FGridParameters
{
    float GridSpacing;
    int   NumGridLines;
    FVector2D Padding1;
    
    FVector GridOrigin;
    float pad;
};

struct FSimpleVertex
{
    float dummy; // 내용은 사용되지 않음
    float padding[11];
};

struct FOBB {
    FVector4 corners[8];
};

struct FRect
{
    FRect() : TopLeftX(0), TopLeftY(0), Width(0), Height(0) {}
    FRect(float x, float y, float w, float h) : TopLeftX(x), TopLeftY(y), Width(w), Height(h) {}
    float TopLeftX, TopLeftY, Width, Height;
};

struct FPoint
{
    FPoint() : x(0), y(0) {}
    FPoint(float _x, float _y) : x(_x), y(_y) {}
    FPoint(long _x, long _y) : x(_x), y(_y) {}
    FPoint(int _x, int _y) : x(_x), y(_y) {}

    float x, y;
};

struct FBoundingBox
{
    FBoundingBox() = default;
    FBoundingBox(FVector _min, FVector _max) : min(_min), max(_max) {}
    FVector min; // Minimum extents
    float pad;
    FVector max; // Maximum extents
    float pad1;
    bool Intersect(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const
    {
        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        constexpr float epsilon = 1e-6f;

        // X축 처리
        if (fabs(rayDir.X) < epsilon)
        {
            // 레이가 X축 방향으로 거의 평행한 경우,
            // 원점의 x가 박스 [min.X, max.X] 범위 밖이면 교차 없음
            if (rayOrigin.X < min.X || rayOrigin.X > max.X)
                return false;
        }
        else
        {
            float t1 = (min.X - rayOrigin.X) / rayDir.X;
            float t2 = (max.X - rayOrigin.X) / rayDir.X;
            if (t1 > t2)  std::swap(t1, t2);

            // tmin은 "현재까지의 교차 구간 중 가장 큰 min"
            tmin = (t1 > tmin) ? t1 : tmin;
            // tmax는 "현재까지의 교차 구간 중 가장 작은 max"
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Y축 처리
        if (fabs(rayDir.Y) < epsilon)
        {
            if (rayOrigin.Y < min.Y || rayOrigin.Y > max.Y)
                return false;
        }
        else
        {
            float t1 = (min.Y - rayOrigin.Y) / rayDir.Y;
            float t2 = (max.Y - rayOrigin.Y) / rayDir.Y;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Z축 처리
        if (fabs(rayDir.Z) < epsilon)
        {
            if (rayOrigin.Z < min.Z || rayOrigin.Z > max.Z)
                return false;
        }
        else
        {
            float t1 = (min.Z - rayOrigin.Z) / rayDir.Z;
            float t2 = (max.Z - rayOrigin.Z) / rayDir.Z;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // 여기까지 왔으면 교차 구간 [tmin, tmax]가 유효하다.
        // tmax < 0 이면, 레이가 박스 뒤쪽에서 교차하므로 화면상 보기엔 교차 안 한다고 볼 수 있음
        if (tmax < 0.0f)
            return false;

        // outDistance = tmin이 0보다 크면 그게 레이가 처음으로 박스를 만나는 지점
        // 만약 tmin < 0 이면, 레이의 시작점이 박스 내부에 있다는 의미이므로, 거리를 0으로 처리해도 됨.
        outDistance = (tmin >= 0.0f) ? tmin : 0.0f;

        return true;
    }
};

struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];
};

struct FPrimitiveCounts
{
    int BoundingBoxCount;
    int pad;
    int ConeCount;
    int pad1;
};

#define MAX_LIGHTS 16
#define NUM_FACES 6
#define MAX_CASCADE_NUM 5

enum ELightType {
    POINT_LIGHT = 1,
    SPOT_LIGHT = 2,
    DIRECTIONAL_LIGHT = 3,
    AMBIENT_LIGHT = 4,
    NUM_LIGHT_TYPES = 5
};

struct FMaterialConstants
{
    FVector DiffuseColor;
    float TransparencyScalar;

    FVector SpecularColor;
    float SpecularScalar;

    FVector EmissiveColor;
    float DensityScalar;

    FVector AmbientColor;
    uint32 TextureFlag;
};

struct FPointLightGSBuffer
{
    FMatrix World;
    FMatrix ViewProj[NUM_FACES]; // 6 : NUM_FACES
};

struct FCascadeConstantBuffer
{
    FMatrix World;
    FMatrix ViewProj[MAX_CASCADE_NUM];
    FMatrix InvViewProj[MAX_CASCADE_NUM];
    FMatrix InvProj[MAX_CASCADE_NUM];
    FVector4 CascadeSplit;

    float pad1;
    float pad2;
};

struct FShadowConstantBuffer
{
    FMatrix ShadowViewProj; // Light 광원 입장에서의 ViewProj
};

struct FObjectConstantBuffer
{
    FMatrix WorldMatrix;
    FMatrix InverseTransposedWorld;
    
    FVector4 UUIDColor;
    
    int bIsSelected;
    FVector pad;
};

struct FCameraConstantBuffer
{
    FMatrix ViewMatrix;
    FMatrix InvViewMatrix;
    
    FMatrix ProjectionMatrix;
    FMatrix InvProjectionMatrix;
    
    FVector ViewLocation;
    float Padding1;

    float NearClip;
    float FarClip;
    FVector2D Padding2;
};

struct FSubUVConstant
{
    FVector2D uvOffset;
    FVector2D uvScale;
};

struct FLitUnlitConstants
{
    int bIsLit; // 1 = Lit, 0 = Unlit 
    FVector pad;
};

struct FIsShadowConstants
{
    int bIsShadow;
    FVector pad;
};

struct FViewModeConstants
{
    uint32 ViewMode;
    FVector pad;
};

struct FSubMeshConstants
{
    float bIsSelectedSubMesh;
    FVector pad;
};

struct FTextureUVConstants
{
    float UOffset;
    float VOffset;
    float pad0;
    float pad1;
};

struct FLinePrimitiveBatchArgs
{
    FGridParameters GridParam;
    ID3D11Buffer* VertexBuffer;
    int BoundingBoxCount;
    int ConeCount;
    int ConeSegmentCount;
    int OBBCount;
};

struct FViewportSize
{
    FVector2D ViewportSize;
    FVector2D Padding;
};

struct FVertexInfo
{
    uint32_t NumVertices;
    uint32_t Stride;
    ID3D11Buffer* VertexBuffer;
};

struct FIndexInfo
{
    uint32_t NumIndices;
    ID3D11Buffer* IndexBuffer;
};

struct FBufferInfo
{
    FVertexInfo VertexInfo;
    FIndexInfo IndexInfo;
};

struct FScreenConstants
{
    FVector2D ScreenSize;   // 화면 전체 크기 (w, h)
    FVector2D UVOffset;     // 뷰포트 시작 UV (x/sw, y/sh)
    FVector2D UVScale;      // 뷰포트 크기 비율 (w/sw, h/sh)
    FVector2D Padding;      // 정렬용 (사용 안 해도 무방)
};

struct FFogConstants
{
    FLinearColor FogColor;
    
    float StartDistance;
    float EndDistance;    
    float FogHeight;
    float FogHeightFalloff;
    
    float FogDensity;
    float FogDistanceWeight;
    float padding1;
    float padding2;
};

struct FFadeConstants
{
    FLinearColor FadeColor = FLinearColor(0,0,0,0);
    float FadeAlpha = 1.0f;
    FVector padding1;
};


namespace FBX
{
    struct FFbxMaterialInfo
    {
        // --- 식별 정보 ---
        FName MaterialName = NAME_None;
        FString UniqueID;

        // --- 기본 재질 속성 ---
        FLinearColor BaseColorFactor = FLinearColor::White;    // PBR: 베이스 컬러 값 (Diffuse 역할도 겸함)
        FLinearColor EmissiveFactor = FLinearColor::Black;     // 자체 발광 색상 값
        FVector SpecularFactor = FVector(1.0f, 1.0f, 1.0f);   // 전통: Specular 색상 계수 (PBR에서는 잘 안씀)
        float MetallicFactor = 0.0f;       // PBR: 금속성 값 (0=비금속, 1=금속)
        float RoughnessFactor = 0.8f;      // PBR: 거칠기 값 (0=매끄러움, 1=거침)
        float SpecularPower = 32.0f;     // 전통: Shininess/Specular Exponent (Roughness의 역과 유사)
        float OpacityFactor = 1.0f;        // 불투명도 (1.0 = 완전 불투명)
        float IOR = 1.5f;                // Index of Refraction (굴절률) - 고급 렌더링용

        // --- 텍스처 경로 ---

        FWString BaseColorTexturePath;      // Diffuse/Albedo 텍스처
        FWString NormalTexturePath;         // 노멀맵 텍스처
        FWString MetallicTexturePath;       // 금속성 텍스처 (종종 Roughness, AO와 채널 패킹됨)
        FWString RoughnessTexturePath;      // 거칠기 텍스처 (종종 Metallic, AO와 채널 패킹됨)
        FWString SpecularTexturePath;       // 전통: Specular 텍스처 (PBR에서는 잘 안씀)
        FWString EmissiveTexturePath;       // 자체 발광 텍스처
        FWString AmbientOcclusionTexturePath; // 앰비언트 오클루전(AO) 텍스처
        FWString OpacityTexturePath;        // 불투명도/알파 마스크 텍스처

        // --- 상태 플래그 ---
        bool bHasBaseColorTexture = false;
        bool bHasNormalTexture = false;
        bool bHasMetallicTexture = false;
        bool bHasRoughnessTexture = false;
        bool bHasSpecularTexture = false;
        bool bHasEmissiveTexture = false;
        bool bHasAmbientOcclusionTexture = false;
        bool bHasOpacityTexture = false;

        bool bIsTransparent = false;
        bool bUsePBRWorkflow = true;

        FFbxMaterialInfo() = default; // 기본 생성자
    };

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
                if (BoneIndices[i] != Other.BoneIndices[i] || !FMath::IsNearlyEqual(BoneWeights[i], Other.BoneWeights[i]))
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

    struct FBoneInfo
    {
        FName Name;
        int32 ParentIndex;
        FMatrix InverseBindPoseMatrix;
        FMatrix BindPoseMatrix;
        FbxNode* Node = nullptr;

        FMatrix CurrentLocalMatrix;
        FMatrix CurrentWorldTransform;

        FBoneInfo() : ParentIndex(-1), CurrentLocalMatrix(FMatrix::Identity), CurrentWorldTransform(FMatrix::Identity) {} // 행렬 초기화
    };

    struct FSkeletalMeshRenderData
    {
        FString MeshName; // 메시 이름 (FBX 노드 이름 등)
        FString FilePath; // 원본 파일 경로 (식별용)

        // --- 핵심 데이터 ---
        TArray<FBX::FMeshVertex> BindPoseVertices;
        TArray<uint32> Indices;

        // --- 스켈레톤 데이터 ---
        TArray<FBX::FBoneInfo> Skeleton;
        TMap<FName, uint32_t> BoneNameToIndexMap; // 본 이름 -> 인덱스 맵 (빠른 검색용)

        // --- 재질 데이터 ---
        TArray<FBX::FFbxMaterialInfo> Materials;      // 이 메시에 사용된 재질 목록
        // 참고: 현재 FLoaderFBX는 첫 번째 메시만 로드하므로, 재질 목록도 해당 메시의 것만 포함됨.
        // OBJ처럼 서브셋별 재질이 필요하면 로더 로직 수정 필요.

        // --- DirectX 버퍼 ---
        ID3D11Buffer* DynamicVertexBuffer = nullptr; // 정점 버퍼 (스키닝 위해 동적)
        ID3D11Buffer* IndexBuffer = nullptr;         // 인덱스 버퍼 (보통 정적)

        // --- 바운딩 박스 ---
        FBoundingBox Bounds; // AABB (Axis-Aligned Bounding Box)

        // --- 생성자 / 소멸자 (리소스 관리) ---
        FSkeletalMeshRenderData() = default;

        ~FSkeletalMeshRenderData()
        {
            ReleaseBuffers(); // 소멸 시 DirectX 버퍼 자동 해제
        }

        // 복사 생성자/대입 연산자 삭제 (버퍼 이중 해제 방지)
        FSkeletalMeshRenderData(const FSkeletalMeshRenderData&) = delete;
        FSkeletalMeshRenderData& operator=(const FSkeletalMeshRenderData&) = delete;

        // 이동 생성자/대입 연산자 허용 (TMap 등에서 효율적)
        FSkeletalMeshRenderData(FSkeletalMeshRenderData&& Other) noexcept
            : MeshName((Other.MeshName)),
            FilePath((Other.FilePath)),
            BindPoseVertices((Other.BindPoseVertices)),
            Indices((Other.Indices)),
            Skeleton((Other.Skeleton)),
            Materials((Other.Materials)),
            DynamicVertexBuffer(Other.DynamicVertexBuffer),
            IndexBuffer(Other.IndexBuffer),
            Bounds(Other.Bounds)
        {
            // 이동 후 원본 객체의 포인터는 nullptr로 설정
            Other.DynamicVertexBuffer = nullptr;
            Other.IndexBuffer = nullptr;
            BoneNameToIndexMap = (Other.BoneNameToIndexMap);
        }

        FSkeletalMeshRenderData& operator=(FSkeletalMeshRenderData&& Other) noexcept
        {
            if (this != &Other)
            {
                ReleaseBuffers(); // 기존 리소스 해제

                MeshName = Other.MeshName;
                FilePath = Other.FilePath;
                BindPoseVertices = Other.BindPoseVertices;
                Indices = (Other.Indices);
                Skeleton = (Other.Skeleton);
                BoneNameToIndexMap = (Other.BoneNameToIndexMap);
                Materials = (Other.Materials);
                DynamicVertexBuffer = Other.DynamicVertexBuffer;
                IndexBuffer = Other.IndexBuffer;
                Bounds = Other.Bounds;

                Other.DynamicVertexBuffer = nullptr;
                Other.IndexBuffer = nullptr;
            }
            return *this;
        }

        // DirectX 버퍼 해제 함수
        void ReleaseBuffers()
        {
            if (DynamicVertexBuffer) { DynamicVertexBuffer->Release(); DynamicVertexBuffer = nullptr; }
            if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = nullptr; }
        }

        // 정점 데이터 로드 후 바운딩 박스 계산 함수
        void CalculateBounds()
        {
            if (BindPoseVertices.IsEmpty())
            {
                Bounds.min = FVector::ZeroVector;
                Bounds.max = FVector::ZeroVector;
                return;
            }

            FVector MinPos = BindPoseVertices[0].Position;
            FVector MaxPos = BindPoseVertices[0].Position;

            for (int32 i = 1; i < BindPoseVertices.Num(); ++i)
            {
                MinPos = FVector::Min(MinPos, BindPoseVertices[i].Position);
                MaxPos = FVector::Max(MaxPos, BindPoseVertices[i].Position);
            }
            Bounds.min = MinPos;
            Bounds.max = MaxPos;
        }
    };

}

