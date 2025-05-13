#include "FBXSceneLoader.h"

#include <filesystem>
#include <functional>

#include "FBXAnimLoader.h"
#include "FBXConversionUtil.h"
#include "FBXManager.h"
#include "FBXMaterialLoader.h"
#include "FBXMeshLoader.h"
#include "FBXSkeletonBuilder.h"
#include "FLoaderFBX.h"

bool FBXSceneLoader::ParseFBX(const FString& FBXFilePath, FBX::FBXInfo& OutFBXInfo)
{
    using namespace FBX;

    // --- FBX SDK 초기화 ---
    SdkManager = FbxManager::Create(); if (!SdkManager) return false;
    FbxIOSettings* IOS = FbxIOSettings::Create(SdkManager, IOSROOT); if (!IOS) return false;
    SdkManager->SetIOSettings(IOS);

    Scene = FbxScene::Create(SdkManager, "ImportScene"); if (!Scene) return false;
    Importer = FbxImporter::Create(SdkManager, ""); if (!Importer) return false;

#if USE_WIDECHAR
    std::string FilepathStdString = FBXFilePath.ToAnsiString();
#else
    std::string FilepathStdString(*FBXFilePath);
#endif
    if (!Importer->Initialize(FilepathStdString.c_str(), -1, SdkManager->GetIOSettings())) return false;
    if (!Importer->Import(Scene)) return false;

    // --- 좌표계 및 단위 변환 ---
    FbxAxisSystem TargetAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
    if (Scene->GetGlobalSettings().GetAxisSystem() != TargetAxisSystem)
        TargetAxisSystem.DeepConvertScene(Scene);
    FbxSystemUnit::m.ConvertScene(Scene);
    FbxGeometryConverter GeometryConverter(SdkManager);
    GeometryConverter.Triangulate(Scene, true);

    // --- 경로 처리 ---
    OutFBXInfo.FilePath = FBXFilePath;
    std::filesystem::path fsPath(FBXFilePath.ToWideString());
    OutFBXInfo.FileDirectory = fsPath.parent_path().wstring().c_str();

    FbxNode* RootNode = Scene->GetRootNode();
    if (!RootNode) return false;

    // --- 재질 파싱 ---
    TMap<FbxSurfaceMaterial*, FName> MaterialPtrToNameMap;
    OutFBXInfo.Materials.Empty();
    int NumTotalNodes = Scene->GetNodeCount();
    for (int nodeIdx = 0; nodeIdx < NumTotalNodes; ++nodeIdx)
    {
        FbxNode* CurrentNode = Scene->GetNode(nodeIdx);
        if (!CurrentNode) continue;

        int MaterialCount = CurrentNode->GetMaterialCount();
        for (int matIdx = 0; matIdx < MaterialCount; ++matIdx)
        {
            FbxSurfaceMaterial* FbxMat = CurrentNode->GetMaterial(matIdx);
            if (FbxMat && !MaterialPtrToNameMap.Contains(FbxMat))
            {
                FName MatName(FbxMat->GetName());
                int suffix = 1;
                FName OriginalName = MatName;
                while (OutFBXInfo.Materials.Contains(MatName))
                    MatName = FName(*(OriginalName.ToString() + FString::Printf(TEXT("_%d"), suffix++)));

                FFbxMaterialInfo MatInfo =  FBX::FBXMaterialLoader::ProcessSingleMaterial(FbxMat, OutFBXInfo.FileDirectory);
                MatInfo.MaterialName = MatName;
                OutFBXInfo.Materials.Add(MatName, MatInfo);
                MaterialPtrToNameMap.Add(FbxMat, MatName);
            }
        }
    }

    // --- 본 계층 추출 ---
    FBXSkeletonBuilder::BuildSkeletonHierarchy(Scene, OutFBXInfo.SkeletonHierarchy, OutFBXInfo.SkeletonRootBoneNames);

    // --- 메시 파싱 ---
    OutFBXInfo.Meshes.Empty();
    std::function<void(FbxNode*)> ProcessNodeRecursive = [&](FbxNode* CurrentNode)
        {
            if (!CurrentNode) return;
            FbxMesh* Mesh = CurrentNode->GetMesh();
            if (Mesh)
            {
                MeshRawData RawData;
                FbxAMatrix GlobalTransform = CurrentNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
                RawData.MeshNodeGlobalTransformAtBindTime = ConvertFbxAMatrixToFMatrix(GlobalTransform);

                if (FBXMeshLoader::ExtractSingleMeshRawData(CurrentNode, RawData, MaterialPtrToNameMap))
                    OutFBXInfo.Meshes.Add(std::move(RawData));
            }

            for (int i = 0; i < CurrentNode->GetChildCount(); ++i)
                ProcessNodeRecursive(CurrentNode->GetChild(i));
        };
    ProcessNodeRecursive(RootNode);

    // --- 디버그: 본 트랜스폼 출력 ---
    for (auto& Pair : OutFBXInfo.SkeletonHierarchy)
    {
        if (Pair.Key.IsNone()) continue;
        const FBoneHierarchyNode& BoneNode = Pair.Value;
        FString BoneInfo = FbxTransformToString(ConvertFMatrixToFbxAMatrix(BoneNode.GlobalBindPose));
        UE_LOG(LogLevel::Display, TEXT("[%s] %s"), *Pair.Key.ToString(), *BoneInfo);
    }

    // --- 애니메이션용 Bone 이름 등록 ---
    TArray<FString> BoneNames;
    for (const auto& Pair : OutFBXInfo.SkeletonHierarchy)
        BoneNames.Add(Pair.Key.ToString());
    FManagerFBX::SetFBXBoneNames(FBXFilePath, BoneNames);

    // --- 애니메이션 파싱 ---
    FBX::FBXAnimLoader::ParseFBXAnim(FBXFilePath, FBXFilePath);

    return true;
}

void FBXSceneLoader::Destroy()
{
    if (Importer)
    {
        Importer->Destroy();
        Importer = nullptr;
    }

    if (Scene)
    {
        Scene->Destroy();
        Scene = nullptr;
    }

    if (SdkManager)
    {
        SdkManager->Destroy();
        SdkManager = nullptr;
    }
}
