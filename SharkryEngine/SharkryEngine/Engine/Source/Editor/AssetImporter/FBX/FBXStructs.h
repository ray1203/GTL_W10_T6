#pragma once

#include "Define.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "UObject/NameTypes.h"
#include <d3d11.h>

namespace FBX
{
    struct MeshRawData;

    struct FBoneHierarchyNode
    {
        FName BoneName;
        FName ParentName;
        FMatrix GlobalBindPose;
        FMatrix TransformMatrix;
    };

    struct FFbxMaterialInfo
    {
        FName MaterialName = NAME_None;
        FString UniqueID;

        FLinearColor BaseColorFactor = FLinearColor::White;
        FLinearColor EmissiveFactor = FLinearColor::Black;
        FVector SpecularFactor = FVector(1.0f, 1.0f, 1.0f);
        float MetallicFactor = 0.0f;
        float RoughnessFactor = 0.8f;
        float SpecularPower = 32.0f;
        float OpacityFactor = 1.0f;

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
        bool bUsePBRWorkflow = true;
    };

    struct FSkeletalMeshVertex
    {
        FVector Position;
        float R, G, B, A;
        FVector Normal;
        float TangentX, TangentY, TangentZ;
        FVector2D TexCoord;
        uint32 MaterialIndex;

        uint32 BoneIndices[MAX_BONE_INFLUENCES] = { 0 };
        float BoneWeights[MAX_BONE_INFLUENCES] = { 0.0f };

        bool operator==(const FSkeletalMeshVertex& Other) const
        {
            if (Position != Other.Position || Normal != Other.Normal || TexCoord != Other.TexCoord)
                return false;

            for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
            {
                if (BoneIndices[i] != Other.BoneIndices[i] ||
                    !FMath::IsNearlyEqual(BoneWeights[i], Other.BoneWeights[i]))
                    return false;
            }
            return true;
        }

        bool operator!=(const FSkeletalMeshVertex& Other) const
        {
            return !(*this == Other);
        }
    };

    struct FMeshSubset
    {
        uint32 IndexStart = 0;
        uint32 IndexCount = 0;
        uint32 MaterialIndex = 0;
    };

    struct FSkeletalMeshInstanceRenderData
    {
        ID3D11Buffer* DynamicVertexBuffer_CPU = nullptr;
        bool bUseGpuSkinning = true;

        ~FSkeletalMeshInstanceRenderData() { ReleaseBuffers(); }

        void ReleaseBuffers()
        {
            if (DynamicVertexBuffer_CPU)
            {
                DynamicVertexBuffer_CPU->Release();
                DynamicVertexBuffer_CPU = nullptr;
            }
        }
    };

    struct FSkeletalMeshRenderData
    {
        FString MeshName;
        FString FilePath;

        TArray<FSkeletalMeshVertex> BindPoseVertices;
        TArray<uint32> Indices;
        TArray<FFbxMaterialInfo> Materials;
        TArray<FMeshSubset> Subsets;

        ID3D11Buffer* SharedVertexBuffer = nullptr;
        ID3D11Buffer* IndexBuffer = nullptr;

        FBoundingBox Bounds;

        FSkeletalMeshRenderData() = default;
        ~FSkeletalMeshRenderData() { ReleaseBuffers(); }

        FSkeletalMeshRenderData(const FSkeletalMeshRenderData&) = delete;
        FSkeletalMeshRenderData& operator=(const FSkeletalMeshRenderData&) = delete;

        FSkeletalMeshRenderData(FSkeletalMeshRenderData&& Other) noexcept;
        FSkeletalMeshRenderData& operator=(FSkeletalMeshRenderData&& Other) noexcept;

        void ReleaseBuffers();
        void CalculateBounds();
    };
    struct FBXInfo
    {
        FString FilePath;
        FWString FileDirectory;
        TArray<FBX::MeshRawData> Meshes;
        TMap<FName, FBX::FBoneHierarchyNode> SkeletonHierarchy;
        TArray<FName> SkeletonRootBoneNames;
        TMap<FName, FBX::FFbxMaterialInfo> Materials;
    };
    struct MeshRawData
    {
        FName NodeName;
        TArray<FVector> ControlPoints;
        TArray<int32> PolygonVertexIndices;
        FMatrix MeshNodeGlobalTransformAtBindTime;
        struct AttributeData {
            TArray<FbxVector4> DataVec4;
            TArray<FbxVector2> DataVec2;
            TArray<int> IndexArray;
            FbxLayerElement::EMappingMode MappingMode = FbxLayerElement::eNone;
            FbxLayerElement::EReferenceMode ReferenceMode = FbxLayerElement::eDirect;
        };

        AttributeData NormalData;
        AttributeData UVData;

        struct RawInfluence {
            FName BoneName;
            TArray<int32> ControlPointIndices;
            TArray<double> ControlPointWeights;
        };

        TArray<RawInfluence> SkinningInfluences;

        TArray<FName> MaterialNames;

        struct MaterialMappingInfo {
            FbxLayerElement::EMappingMode MappingMode = FbxLayerElement::eNone;
            TArray<int32> IndexArray;
        } MaterialMapping;
    };
}

// std::hash 특수화
namespace std
{
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <>
    struct hash<FBX::FSkeletalMeshVertex>
    {
        size_t operator()(const FBX::FSkeletalMeshVertex& Key) const noexcept;
    };
}
