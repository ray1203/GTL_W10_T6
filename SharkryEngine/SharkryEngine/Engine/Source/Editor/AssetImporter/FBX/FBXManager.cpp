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

#include "Engine/Source/ThirdParty/JSON/json.hpp"
#include <fstream>
#include "Engine/Source/Runtime/Engine/Animation/AnimNotify.h"
using json = nlohmann::json;

TMap<FString, TArray<FAnimNotifyEvent>> FManagerFBX::SequenceMap;

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

UAnimationAsset* FManagerFBX::GetAnimationAsset(const FString& Name)
{
    if (AnimationAssetMap.Contains(Name)) return AnimationAssetMap[Name];
    return nullptr;
}

void FManagerFBX::AddAnimationAsset(const FString& Name, UAnimationAsset* AnimationAsset)
{
    AnimationAssetMap[Name] = AnimationAsset;
}

void FManagerFBX::CreateAnimationAsset(const FWString& Name)
{
    if (AnimationAssetMap.Contains(FString(Name.c_str()))) return;

    FBX::FBXAnimLoader::ParseFBXAnim(FString(Name.c_str()));
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

void FManagerFBX::AddNotifySequence(const FString& SequenceName, const TArray<FAnimNotifyEvent>& Events)
{
    SequenceMap.Add(SequenceName, Events);
}

bool FManagerFBX::SaveNotifySequencesJson(const std::filesystem::path& FilePath)
{
    // 1) 전달받은 FilePath가 절대 경로인지 확인
    std::filesystem::path fullPath = FilePath;
    if (fullPath.is_relative())
    {
        // 실행 중인 EXE의 위치를 기준으로 삼고 싶으면 GetModuleFileNameW 사용
        WCHAR exeBuf[MAX_PATH];
        GetModuleFileNameW(nullptr, exeBuf, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();

        fullPath = exeDir / FilePath;
        // 또는 현재 작업 디렉터리 기준으로 삼으려면
        // fullPath = std::filesystem::current_path() / FilePath;
    }

    // 2) 필요한 디렉터리(중간 경로) 생성
    auto Dir = fullPath.parent_path();
    if (!std::filesystem::exists(Dir))
    {
        std::filesystem::create_directories(Dir);
    }

    // 3) JSON 직렬화
    json j;
    for (auto& [Key, Events] : SequenceMap)
    {
        std::string seqName = *Key;
        json arr = json::array();
        for (auto& Evt : Events)
        {
            arr.push_back({
                {"TriggerTime",         Evt.TriggerTime},
                {"TriggerFrame",        Evt.TriggerFrame},
                {"Duration",            Evt.Duration},
                {"TriggerEndFrame",     Evt.TriggerEndFrame},
                {"isTriggerEndClicked", Evt.isTriggerEndClicked},
                {"NotifyName",          *Evt.NotifyName.ToString()},
                {"NotifyMode",          static_cast<int>(Evt.NotifyMode)},
                {"NotifyState",         static_cast<int>(Evt.NotifyState)},
                {"TrackNum",            Evt.TrackNum}
                });
        }
        j["Sequences"][seqName] = std::move(arr);
    }

    // 4) 파일 쓰기
    std::ofstream ofs(fullPath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
        std::string msg = "Failed to open file for writing: " + fullPath.string();
        MessageBoxA(nullptr, msg.c_str(), "Error", MB_OK | MB_ICONERROR);
        return false;
    }
    ofs << j.dump(4);
    return true;
}


bool FManagerFBX::LoadNotifySequencesJson(const FString& FilePath)
{
    // 1) std::ifstream 으로 파일 읽기
    std::ifstream ifs(*FilePath);
    if (!ifs.is_open())
    {
        UE_LOG(LogLevel::Warning, TEXT("Cannot open file for reading: %s"), *FilePath);
        return false;
    }

    // 2) std::string에 전체 스트림 읽어들이기
    std::string inStr((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    // 3) nlohmann::json 파싱
    json j = json::parse(inStr, /*cb=*/nullptr, /*allow_exceptions=*/false);
    if (j.is_discarded())
    {
        UE_LOG(LogLevel::Warning, TEXT("Invalid JSON in %s"), *FilePath);
        return false;
    }

    // 4) 역직렬화
    SequenceMap.Empty();
    if (j.contains("Sequences") && j["Sequences"].is_object())
    {
        for (auto& [seqNameStd, arr] : j["Sequences"].items())
        {
            // UTF-8 std::string ➔ FString
            FString seqName = seqNameStd.c_str();

            TArray<FAnimNotifyEvent> events;
            for (auto& item : arr)
            {
                FAnimNotifyEvent Evt;
                Evt.TriggerTime = item.at("TriggerTime").get<float>();
                Evt.TriggerFrame = item.at("TriggerFrame").get<int>();
                Evt.Duration = item.at("Duration").get<float>();
                Evt.TriggerEndFrame = item.at("TriggerEndFrame").get<int>();
                Evt.isTriggerEndClicked = item.at("isTriggerEndClicked").get<bool>();
                Evt.NotifyName = FName(item.at("NotifyName").get<std::string>().c_str());
                Evt.NotifyMode = static_cast<ENotifyMode>(item.at("NotifyMode").get<int>());
                Evt.NotifyState = static_cast<ENotifyState>(item.at("NotifyState").get<int>());
                Evt.TrackNum = item.at("TrackNum").get<int>();

                events.Add(Evt);
            }

            SequenceMap.Add(seqName, events);
        }
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("'Sequences' object missing in %s"), *FilePath);
    }

    return true;
}
