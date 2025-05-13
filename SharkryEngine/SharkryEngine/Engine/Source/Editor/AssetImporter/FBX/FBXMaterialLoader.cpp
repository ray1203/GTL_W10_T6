#include "FBXMaterialLoader.h"
#include "FBXConversionUtil.h" // FVector / FLinearColor 변환 등
#include <fbxsdk.h>
#include <filesystem>

FWString FBX::FBXMaterialLoader::ProcessTexturePathInternal(FbxFileTexture* Texture, const FWString& BaseDirectory)
{
    if (!Texture) return FWString();
    const char* RelativePathAnsi = Texture->GetRelativeFileName();
    if (RelativePathAnsi && RelativePathAnsi[0] != '\0' && !BaseDirectory.empty())
    {
        FString RelativePath(RelativePathAnsi);
        std::filesystem::path BaseDirPath(BaseDirectory);
        std::filesystem::path RelPath(RelativePath.ToWideString());
        std::error_code ec;
        std::filesystem::path CombinedPath = BaseDirPath / RelPath;
        CombinedPath = std::filesystem::absolute(CombinedPath, ec);
        if (!ec) {
            CombinedPath.make_preferred();
            if (std::filesystem::exists(CombinedPath, ec) && !ec) return FWString(CombinedPath.wstring().c_str());
            ec.clear();
        }
        else { ec.clear(); }
    }
    const char* AbsolutePathAnsi = Texture->GetFileName();
    if (AbsolutePathAnsi && AbsolutePathAnsi[0] != '\0')
    {
        FString AbsolutePath(AbsolutePathAnsi);
        std::filesystem::path AbsPath(AbsolutePath.ToWideString());
        std::error_code ec;
        AbsPath.make_preferred();
        if (std::filesystem::exists(AbsPath, ec) && !ec) return FWString(AbsPath.wstring().c_str());
        ec.clear();
    }
    const char* PathToExtractFromAnsi = (AbsolutePathAnsi && AbsolutePathAnsi[0] != '\0') ? AbsolutePathAnsi : RelativePathAnsi;
    if (PathToExtractFromAnsi && PathToExtractFromAnsi[0] != '\0') {
        FString PathToExtractFrom(PathToExtractFromAnsi);
        std::filesystem::path TempPath(PathToExtractFrom.ToWideString());
        if (TempPath.has_filename()) return FWString(TempPath.filename().wstring().c_str());
    }
    return FWString();
}

FBX::FFbxMaterialInfo FBX::FBXMaterialLoader::ProcessSingleMaterial(FbxSurfaceMaterial* FbxMaterial, const FWString& BaseDirectory) {
    FFbxMaterialInfo MatInfo;
    if (!FbxMaterial) return MatInfo;
    MatInfo.MaterialName = FName(FbxMaterial->GetName());
    // ... (Full material property and texture extraction logic from previous answers) ...
    // --- Property Names ---
    const char* DiffuseColorPropName = FbxSurfaceMaterial::sDiffuse;
    const char* DiffuseFactorPropName = FbxSurfaceMaterial::sDiffuseFactor;
    const char* BaseColorPropName = "BaseColor";
    const char* NormalMapPropName = FbxSurfaceMaterial::sNormalMap;
    const char* BumpMapPropName = FbxSurfaceMaterial::sBump;
    const char* MetallicPropName = "Metallic";
    const char* MetalnessPropName = "Metalness";
    const char* RoughnessPropName = "Roughness";
    const char* SpecularColorPropName = FbxSurfaceMaterial::sSpecular;
    const char* SpecularFactorPropName = FbxSurfaceMaterial::sSpecularFactor;
    const char* ShininessPropName = FbxSurfaceMaterial::sShininess;
    const char* EmissiveColorPropName = FbxSurfaceMaterial::sEmissive;
    const char* EmissiveFactorPropName = FbxSurfaceMaterial::sEmissiveFactor;
    const char* AmbientOcclusionPropName = "AmbientOcclusion";
    const char* OcclusionPropName = "Occlusion";
    const char* OpacityPropName = FbxSurfaceMaterial::sTransparencyFactor;

    auto ExtractTextureInternal = [&](const char* PropName, FWString& OutPath, bool& bOutHasTexture) {
        FbxProperty Property = FbxMaterial->FindProperty(PropName);
        bOutHasTexture = false;
        OutPath.clear();

        if (Property.IsValid())
        {
            int LayeredTextureCount = Property.GetSrcObjectCount<FbxLayeredTexture>();
            FbxFileTexture* Texture = nullptr;
            if (LayeredTextureCount > 0)
            {
                FbxLayeredTexture* LayeredTexture = Property.GetSrcObject<FbxLayeredTexture>(0);
                if (LayeredTexture && LayeredTexture->GetSrcObjectCount<FbxFileTexture>() > 0)
                    Texture = LayeredTexture->GetSrcObject<FbxFileTexture>(0);
            }
            else if (Property.GetSrcObjectCount<FbxFileTexture>() > 0)
            {
                Texture = Property.GetSrcObject<FbxFileTexture>(0);
            }
            if (Texture)
            {
                OutPath = ProcessTexturePathInternal(Texture, BaseDirectory);
                bOutHasTexture = !OutPath.empty();
            }
        }
        };

    FbxProperty BaseColorProp = FbxMaterial->FindProperty(BaseColorPropName);
    if (!BaseColorProp.IsValid()) BaseColorProp = FbxMaterial->FindProperty(DiffuseColorPropName);
    if (BaseColorProp.IsValid()) {
        MatInfo.BaseColorFactor = ConvertFbxColorToLinear(BaseColorProp.Get<FbxDouble3>());
        FbxProperty DiffuseFactorProp = FbxMaterial->FindProperty(DiffuseFactorPropName);
        if (DiffuseFactorProp.IsValid()) MatInfo.BaseColorFactor *= static_cast<float>(DiffuseFactorProp.Get<FbxDouble>());
    }
    else { MatInfo.BaseColorFactor = FLinearColor::White; }
    ExtractTextureInternal(BaseColorPropName, MatInfo.BaseColorTexturePath, MatInfo.bHasBaseColorTexture);
    if (!MatInfo.bHasBaseColorTexture) ExtractTextureInternal(DiffuseColorPropName, MatInfo.BaseColorTexturePath, MatInfo.bHasBaseColorTexture);

    ExtractTextureInternal(NormalMapPropName, MatInfo.NormalTexturePath, MatInfo.bHasNormalTexture);
    if (!MatInfo.bHasNormalTexture) ExtractTextureInternal(BumpMapPropName, MatInfo.NormalTexturePath, MatInfo.bHasNormalTexture);

    FbxProperty EmissiveProp = FbxMaterial->FindProperty(EmissiveColorPropName);
    if (EmissiveProp.IsValid()) {
        MatInfo.EmissiveFactor = ConvertFbxColorToLinear(EmissiveProp.Get<FbxDouble3>());
        FbxProperty EmissiveFactorProp = FbxMaterial->FindProperty(EmissiveFactorPropName);
        if (EmissiveFactorProp.IsValid()) MatInfo.EmissiveFactor *= static_cast<float>(EmissiveFactorProp.Get<FbxDouble>());
    }
    else { MatInfo.EmissiveFactor = FLinearColor::Black; }
    ExtractTextureInternal(EmissiveColorPropName, MatInfo.EmissiveTexturePath, MatInfo.bHasEmissiveTexture);

    FbxProperty OpacityProp = FbxMaterial->FindProperty(OpacityPropName);
    MatInfo.OpacityFactor = 1.0f;
    if (OpacityProp.IsValid()) MatInfo.OpacityFactor = 1.0f - static_cast<float>(OpacityProp.Get<FbxDouble>());
    // ExtractTextureInternal("Opacity", MatInfo.OpacityTexturePath, MatInfo.bHasOpacityTexture);

    FbxProperty MetallicProp = FbxMaterial->FindProperty(MetallicPropName);
    if (!MetallicProp.IsValid()) MetallicProp = FbxMaterial->FindProperty(MetalnessPropName);
    FbxProperty RoughnessProp = FbxMaterial->FindProperty(RoughnessPropName);
    FWString MetallicTexPath, RoughnessTexPath, AOTexPath, SpecularTexPath;
    bool bHasMetallicTex, bHasRoughnessTex, bHasAOTex, bHasSpecularTex;
    ExtractTextureInternal(MetallicPropName, MetallicTexPath, bHasMetallicTex);
    if (!bHasMetallicTex) ExtractTextureInternal(MetalnessPropName, MetallicTexPath, bHasMetallicTex);
    ExtractTextureInternal(RoughnessPropName, RoughnessTexPath, bHasRoughnessTex);
    ExtractTextureInternal(AmbientOcclusionPropName, AOTexPath, bHasAOTex);
    if (!bHasAOTex) ExtractTextureInternal(OcclusionPropName, AOTexPath, bHasAOTex);
    ExtractTextureInternal(SpecularColorPropName, SpecularTexPath, bHasSpecularTex);

    if (MetallicProp.IsValid() || RoughnessProp.IsValid() || bHasMetallicTex || bHasRoughnessTex) {
        MatInfo.bUsePBRWorkflow = true;
        MatInfo.MetallicFactor = MetallicProp.IsValid() ? static_cast<float>(MetallicProp.Get<FbxDouble>()) : 0.0f;
        MatInfo.MetallicTexturePath = MetallicTexPath; MatInfo.bHasMetallicTexture = bHasMetallicTex;
        MatInfo.RoughnessFactor = RoughnessProp.IsValid() ? static_cast<float>(RoughnessProp.Get<FbxDouble>()) : 0.8f;
        MatInfo.RoughnessTexturePath = RoughnessTexPath; MatInfo.bHasRoughnessTexture = bHasRoughnessTex;
        MatInfo.AmbientOcclusionTexturePath = AOTexPath; MatInfo.bHasAmbientOcclusionTexture = bHasAOTex;
        MatInfo.SpecularFactor = FVector(0.04f, 0.04f, 0.04f); MatInfo.SpecularPower = 0.0f;
    }
    else {
        MatInfo.bUsePBRWorkflow = false;
        FbxProperty SpecularColorProp = FbxMaterial->FindProperty(SpecularColorPropName);
        FbxProperty SpecularFactorProp = FbxMaterial->FindProperty(SpecularFactorPropName);
        MatInfo.SpecularFactor = FVector(1.0f, 1.0f, 1.0f);
        if (SpecularColorProp.IsValid()) MatInfo.SpecularFactor = ConvertFbxColorToLinear(SpecularColorProp.Get<FbxDouble3>()).ToVector3();
        if (SpecularFactorProp.IsValid()) MatInfo.SpecularFactor *= static_cast<float>(SpecularFactorProp.Get<FbxDouble>());
        MatInfo.SpecularTexturePath = SpecularTexPath; MatInfo.bHasSpecularTexture = bHasSpecularTex;
        FbxProperty ShininessProp = FbxMaterial->FindProperty(ShininessPropName);
        MatInfo.SpecularPower = 32.0f;
        if (ShininessProp.IsValid()) MatInfo.SpecularPower = static_cast<float>(ShininessProp.Get<FbxDouble>());
        MatInfo.RoughnessFactor = FMath::Clamp(FMath::Sqrt(2.0f / (MatInfo.SpecularPower + 2.0f)), 0.0f, 1.0f);
        MatInfo.MetallicFactor = 0.0f;
    }
    MatInfo.bIsTransparent = (MatInfo.OpacityFactor < 1.0f - KINDA_SMALL_NUMBER) || MatInfo.bHasOpacityTexture;
    return MatInfo;
}
void FBX::FBXMaterialLoader::ConvertFbxMaterialToObjMaterial(const FFbxMaterialInfo& FbxInfo, FObjMaterialInfo& OutObjInfo)
{
    // --- 기본 정보 매핑 ---
    OutObjInfo.MaterialName = FbxInfo.MaterialName.ToString(); // FName -> FString
    OutObjInfo.bTransparent = FbxInfo.bIsTransparent;

    // --- 색상 매핑 ---
    OutObjInfo.Diffuse = FbxInfo.BaseColorFactor.ToVector3(); // BaseColor -> Diffuse (Kd)
    OutObjInfo.Emissive = FbxInfo.EmissiveFactor.ToVector3(); // Emissive -> Emissive (Ke)

    // --- 워크플로우에 따른 Specular 및 Shininess 매핑 ---
    if (FbxInfo.bUsePBRWorkflow)
    {
        // PBR -> Traditional 근사 변환
        // Ks (Specular Color): PBR에서는 직접적인 대응 없음. 금속성 여부에 따라 다름.
        // 단순화를 위해 비금속 기본 반사율(F0=0.04) 또는 중간 회색 사용.
        // 또는 BaseColor를 사용할 수도 있음. 여기서는 중간 회색 사용.
        OutObjInfo.Specular = FVector(0.5f, 0.5f, 0.5f);

        // Ns (Shininess/Specular Power): Roughness에서 변환 (근사치)
        // Roughness 0 (매끈) -> Ns 높음, Roughness 1 (거침) -> Ns 낮음
        // 예시: Roughness를 [0,1] -> Glossiness [1,0] -> Ns [~1000, ~2] 범위로 매핑 시도
        float Glossiness = 1.0f - FbxInfo.RoughnessFactor;
        // 비선형 매핑 (값이 너무 커지지 않도록 조정 가능)
        OutObjInfo.SpecularScalar = FMath::Clamp(2.0f * FMath::Pow(100.0f, Glossiness), 2.0f, 1000.0f);
    }
    else
    {
        // Traditional 정보 직접 사용
        OutObjInfo.Specular = FbxInfo.SpecularFactor;     // Ks
        OutObjInfo.SpecularScalar = FbxInfo.SpecularPower; // Ns
    }

    // Ka (Ambient Color): 보통 Diffuse의 일부 또는 작은 기본값 사용
    OutObjInfo.Ambient = OutObjInfo.Diffuse * 0.1f; // Diffuse의 10%를 Ambient로 사용 (예시)

    // --- 스칼라 값 매핑 ---
    // d/Tr (Transparency): OpacityFactor (1=불투명, 0=투명) -> Transparency (1=불투명, 0=투명)
    // OBJ의 d는 불투명도, Tr은 투명도인 경우가 많으므로 주의. 여기서는 d (불투명도) 기준으로 변환.
    OutObjInfo.TransparencyScalar = FbxInfo.OpacityFactor;
    // Ni (Optical Density/IOR): FBX 정보에 직접 매핑되는 값 없음. 기본값 사용.
    OutObjInfo.DensityScalar = 1.0f; // 기본값
    // -bm (Bump Multiplier): FBX 정보에 직접 매핑되는 값 없음. 기본값 사용.
    OutObjInfo.BumpMultiplier = 1.0f; // 기본값
    // illum (Illumination Model): PBR 여부에 따라 기본 모델 선택 가능.
    OutObjInfo.IlluminanceModel = FbxInfo.bUsePBRWorkflow ? 2 : 2; // 예: 기본적으로 Phong 모델(2) 사용

    // --- 텍스처 경로 매핑 ---
    // 이름만 복사, 경로는 그대로 사용
    OutObjInfo.DiffuseTexturePath = FbxInfo.BaseColorTexturePath;
    // FbxInfo.BaseColorTexturePath에서 파일 이름만 추출하여 DiffuseTextureName 설정 (필요 시)
    if (!FbxInfo.BaseColorTexturePath.empty())
    {
        std::filesystem::path p(FbxInfo.BaseColorTexturePath);
        OutObjInfo.DiffuseTextureName = FString(p.filename().string().c_str()); // std::string -> FString
    }
    else { OutObjInfo.DiffuseTextureName.Empty(); }

    OutObjInfo.BumpTexturePath = FbxInfo.NormalTexturePath; // Normal -> Bump
    if (!FbxInfo.NormalTexturePath.empty())
    {
        std::filesystem::path p(FbxInfo.NormalTexturePath);
        OutObjInfo.BumpTextureName = FString(p.filename().string().c_str());
    }
    else { OutObjInfo.BumpTextureName.Empty(); }

    OutObjInfo.SpecularTexturePath = FbxInfo.SpecularTexturePath; // Traditional Specular
    if (!FbxInfo.SpecularTexturePath.empty())
    {
        std::filesystem::path p(FbxInfo.SpecularTexturePath);
        OutObjInfo.SpecularTextureName = FString(p.filename().string().c_str());
    }
    else { OutObjInfo.SpecularTextureName.Empty(); }

    OutObjInfo.AlphaTexturePath = FbxInfo.OpacityTexturePath; // Opacity -> Alpha (map_d)
    if (!FbxInfo.OpacityTexturePath.empty())
    {
        std::filesystem::path p(FbxInfo.OpacityTexturePath);
        OutObjInfo.AlphaTextureName = FString(p.filename().string().c_str());
    }
    else { OutObjInfo.AlphaTextureName.Empty(); }

    // Ambient Texture (map_Ka): FBX AO 맵을 사용
    OutObjInfo.AmbientTexturePath = FbxInfo.AmbientOcclusionTexturePath;
    if (!FbxInfo.AmbientOcclusionTexturePath.empty())
    {
        std::filesystem::path p(FbxInfo.AmbientOcclusionTexturePath);
        OutObjInfo.AmbientTextureName = FString(p.filename().string().c_str());
    }
    else { OutObjInfo.AmbientTextureName.Empty(); }

    // TODO: FObjMaterialInfo에 EmissiveTexturePath/Name 필드 추가 필요 시 매핑
    // OutObjInfo.EmissiveTexturePath = FbxInfo.EmissiveTexturePath;
    // if (!FbxInfo.EmissiveTexturePath.IsEmpty()) { ... }

    // --- TextureFlag 설정 ---
    // FObjMaterialInfo의 TextureFlag 비트 정의에 맞춰 설정
    // 예시: (1 << 1) = Diffuse, (1 << 2) = Bump, (1 << 3) = Specular, (1 << 4) = Alpha, (1 << 5) = Ambient
    OutObjInfo.TextureFlag = 0;
    if (FbxInfo.bHasBaseColorTexture && !FbxInfo.BaseColorTexturePath.empty()) OutObjInfo.TextureFlag |= (1 << 1);
    if (FbxInfo.bHasNormalTexture && !FbxInfo.NormalTexturePath.empty())    OutObjInfo.TextureFlag |= (1 << 2);
    if (FbxInfo.bHasSpecularTexture && !FbxInfo.SpecularTexturePath.empty()) OutObjInfo.TextureFlag |= (1 << 3); // Traditional Specular
    if (FbxInfo.bHasOpacityTexture && !FbxInfo.OpacityTexturePath.empty())   OutObjInfo.TextureFlag |= (1 << 4); // map_d
    if (FbxInfo.bHasAmbientOcclusionTexture && !FbxInfo.AmbientOcclusionTexturePath.empty()) OutObjInfo.TextureFlag |= (1 << 5); // map_Ka (AO 사용)
}
