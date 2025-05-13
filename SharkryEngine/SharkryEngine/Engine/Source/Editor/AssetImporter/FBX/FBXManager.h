// FBXManager.h
#pragma once

#include "Define.h"
#include "FBXStructs.h"
#include "UObject/NameTypes.h"
#include "Container/Array.h"
#include "Container/Map.h"

class USkeletalMesh;
class UMaterial;
class USkeleton;
class UAnimationAsset;

class FManagerFBX
{
public:
    static FBX::FSkeletalMeshRenderData* LoadFBXSkeletalMeshAsset(const FString& PathFileName, USkeleton* OutSkeleton);

    static bool SaveSkeletalMeshToBinary(const FWString& FilePath, const FBX::FSkeletalMeshRenderData& SkeletalMesh);
    static bool LoadSkeletalMeshFromBinary(const FWString& FilePath, FBX::FSkeletalMeshRenderData& OutSkeletalMesh);

    static USkeletalMesh* CreateSkeletalMesh(const FString& FilePath);
    static const TMap<FWString, USkeletalMesh*>& GetSkeletalMeshes();
    static USkeletalMesh* GetSkeletalMesh(const FWString& Name);
    static int GetSkeletalMeshNum();

    static UAnimationAsset* GetAnimationAsset(const FString& name);
    static void AddAnimationAsset(const FString& name, UAnimationAsset* AnimationAsset);
    static TMap<FString, UAnimationAsset*>& GetAnimationAssets() { return AnimationAssetMap; }
    static void CreateAnimationAsset(const FWString& name);

    static UMaterial* CreateMaterial(const FBX::FFbxMaterialInfo& MaterialInfo);
    static TMap<FString, UMaterial*>& GetMaterials();
    static UMaterial* GetMaterial(const FString& Name);
    static int GetMaterialNum();

private:
    inline static TMap<FString, FBX::FSkeletalMeshRenderData*> FBXSkeletalMeshMap;
    inline static TMap<FWString, USkeletalMesh*> SkeletalMeshMap;
    inline static TMap<FString, UMaterial*> MaterialMap;
    inline static TMap<FString, UAnimationAsset*> AnimationAssetMap;
};
