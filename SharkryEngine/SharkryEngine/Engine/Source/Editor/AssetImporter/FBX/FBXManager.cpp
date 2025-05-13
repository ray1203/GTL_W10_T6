// FBXManager.cpp
#include "FBXManager.h"
#include "FBXStructs.h"
#include "FLoaderFBX.h"
#include "UObject/ObjectFactory.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Components/Material/Material.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/AnimDataModel.h"
#include <filesystem>

#include "FBXAnimLoader.h"
#include "FBXMaterialLoader.h"
#include "FBXMeshLoader.h"
#include "FBXSceneLoader.h"
#include "Renderer/DepthPrePass.h"

FBX::FSkeletalMeshRenderData* FManagerFBX::LoadFBXSkeletalMeshAsset(const FString& PathFileName, USkeleton* OutSkeleton)
{
    using namespace FBX;
    if (!OutSkeleton) return nullptr;

    if (FBXSkeletalMeshMap.Contains(PathFileName))
        return FBXSkeletalMeshMap[PathFileName];

    FBXInfo ParsedInfo;
    if (!FBXSceneLoader::ParseFBX(PathFileName, ParsedInfo)) return nullptr;
    if (ParsedInfo.Meshes.IsEmpty()) return nullptr;

    FSkeletalMeshRenderData* NewRenderData = new FSkeletalMeshRenderData();
    if (!FBXMeshLoader::ConvertToSkeletalMesh(ParsedInfo.Meshes, ParsedInfo, *NewRenderData, OutSkeleton))
    {
        delete NewRenderData;
        return nullptr;
    }
    FBXSkeletalMeshMap.Add(PathFileName, NewRenderData);
    return NewRenderData;
}

bool FManagerFBX::SaveSkeletalMeshToBinary(const FWString& FilePath, const FBX::FSkeletalMeshRenderData& SkeletalMesh) { return false; }
bool FManagerFBX::LoadSkeletalMeshFromBinary(const FWString& FilePath, FBX::FSkeletalMeshRenderData& OutSkeletalMesh) { return false; }

USkeletalMesh* FManagerFBX::CreateSkeletalMesh(const FString& FilePath)
{
    FWString MeshKey = FilePath.ToWideString();
    if (SkeletalMeshMap.Contains(MeshKey)) return SkeletalMeshMap[MeshKey];

    USkeletalMesh* NewMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    NewMesh->Skeleton = FObjectFactory::ConstructObject<USkeleton>(NewMesh);
    if (!NewMesh->Skeleton) return nullptr;

    FBX::FSkeletalMeshRenderData* RenderData = LoadFBXSkeletalMeshAsset(FilePath, NewMesh->Skeleton);
    if (!RenderData) return nullptr;

    NewMesh->SetData(RenderData);
    SkeletalMeshMap.Add(MeshKey, NewMesh);
    return NewMesh;
}

const TMap<FWString, USkeletalMesh*>& FManagerFBX::GetSkeletalMeshes() { return SkeletalMeshMap; }
USkeletalMesh* FManagerFBX::GetSkeletalMesh(const FWString& Name)
{
    if (SkeletalMeshMap.Contains(Name)) return SkeletalMeshMap[Name];
    return CreateSkeletalMesh(FString(Name.c_str()));
}

TArray<UAnimationAsset*> FManagerFBX::GetAnimationAssets(const FString& Name)
{
    return AnimationAssetMap.Contains(Name) ? AnimationAssetMap[Name] : TArray<UAnimationAsset*>();
}

void FManagerFBX::AddAnimationAssets(const FString& Name, UAnimationAsset* AnimationAsset)
{
    AnimationAssetMap.FindOrAdd(Name).Add(AnimationAsset);
}

void FManagerFBX::CreateAnimationAsset(const FWString& Name, const FWString& AnimParentFBXFilePath)
{
    FBX::FBXAnimLoader::ParseFBXAnim(FString(Name.c_str()), FString(AnimParentFBXFilePath.c_str()));
}

void FManagerFBX::SetFBXBoneNames(const FString& Name, TArray<FString> Node)
{
    FBXBoneNameMap.Add(Name, Node);
}

TArray<FString> FManagerFBX::GetFBXBoneNames(const FString& Name)
{
    return FBXBoneNameMap.Contains(Name) ? FBXBoneNameMap[Name] : TArray<FString>();
}

UMaterial* FManagerFBX::CreateMaterial(const FBX::FFbxMaterialInfo& MaterialInfo)
{
    FString MatKey = MaterialInfo.MaterialName.ToString();
    if (UMaterial** FoundMat = MaterialMap.Find(MatKey)) return *FoundMat;

    UMaterial* NewMat = FObjectFactory::ConstructObject<UMaterial>(nullptr);
    if (!NewMat) return nullptr;

    FObjMaterialInfo ObjInfo;
    FBX::FBXMaterialLoader::ConvertFbxMaterialToObjMaterial(MaterialInfo, ObjInfo);
    NewMat->SetMaterialInfo(ObjInfo);

    if (MaterialInfo.bHasBaseColorTexture)
        FBXMeshLoader::CreateTextureFromFile(MaterialInfo.BaseColorTexturePath);
    if (MaterialInfo.bHasNormalTexture)
        FBXMeshLoader::CreateTextureFromFile(MaterialInfo.NormalTexturePath);

    MaterialMap.Add(MatKey, NewMat);
    return NewMat;
}

TMap<FString, UMaterial*>& FManagerFBX::GetMaterials() { return MaterialMap; }
UMaterial* FManagerFBX::GetMaterial(const FString& Name) { UMaterial** Ptr = MaterialMap.Find(Name); return Ptr ? *Ptr : nullptr; }
int FManagerFBX::GetMaterialNum() { return MaterialMap.Num(); }
