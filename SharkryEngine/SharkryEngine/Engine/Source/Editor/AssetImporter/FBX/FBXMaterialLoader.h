#pragma once

#include "Define.h"
#include "FBXStructs.h"
#include <fbxsdk.h>

namespace FBX
{
    /// FBX 재질 정보를 FFbxMaterialInfo로 추출하는 유틸리티 클래스
    class FBXMaterialLoader
    {
    public:
        /// 단일 FBX 재질을 엔진 형식(FFbxMaterialInfo)으로 변환
        static FFbxMaterialInfo ProcessSingleMaterial(FbxSurfaceMaterial* FbxMaterial, const FWString& BasePath);

        /// 내부에서 텍스처 경로를 추출하고 확장자 및 절대경로 보정 처리
        static FWString ProcessTexturePathInternal(FbxFileTexture* Texture, const FWString& BaseDirectory);
        static void ConvertFbxMaterialToObjMaterial(const FFbxMaterialInfo& FbxInfo, FObjMaterialInfo& OutObjInfo);

    };
}
