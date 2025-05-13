#pragma once

#include "Define.h"
#include "FBXStructs.h"

#include <fbxsdk.h>

namespace FBX
{
    struct FBXInfo;
}

/// FBX 전체 씬을 로드하고, 내부적으로 메시, 본, 재질, 애니메이션을 추출하는 클래스
class FBXSceneLoader
{
public:
    static FbxManager* GetSdkManager() { return SdkManager; }
    static FbxScene* GetScene() { return Scene; }
    static FbxImporter* GetImporter() { return Importer; }
    /// FBX 파일에서 전체 씬 정보를 파싱합니다 (메시, 본, 재질, 애니메이션 포함)
    static bool ParseFBX(const FString& FBXFilePath, FBX::FBXInfo& OutFBXInfo);

    /// FBX SDK 관련 자원을 해제합니다
    static void Destroy();

private:
    inline static FbxManager* SdkManager = nullptr;
    inline static FbxImporter* Importer = nullptr;
    inline static FbxScene* Scene = nullptr;
};
