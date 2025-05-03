#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"

#include "Define.h"
#include "Components/Light/PointLightComponent.h"

class FShadowManager;
class FDXDShaderManager;
class UWorld;
class UMaterial;
class UStaticMeshComponent;
struct FStaticMaterial;
class FShadowRenderPass;
class FLoaderFBX;
class FSkeletalMeshRenderPass : public IRenderPass
{
public:
    FSkeletalMeshRenderPass();
    
    virtual ~FSkeletalMeshRenderPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    
    void InitializeShadowManager(class FShadowManager* InShadowManager);
    
    virtual void PrepareRenderArr() override;

    virtual void Render(const std::shared_ptr<FViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;
    void RenderAllStaticMeshesForPointLight(const std::shared_ptr<FViewportClient>& Viewport, UPointLightComponent*& PointLight);

    virtual void PrepareRenderState(const std::shared_ptr<FViewportClient>& Viewport);

    virtual void RenderAllSKeletalMeshes(const std::shared_ptr<FViewportClient>& Viewport);
    
    void UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const;
  
    void UpdateLitUnlitConstant(int32 isLit) const;

    void RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const;
    
    // Shader 관련 함수 (생성/해제 등)
    void CreateShader();
    void ReleaseShader();

    void ChangeViewMode(EViewModeIndex ViewModeIndex);
    
protected:


    TArray<UStaticMeshComponent*> SkeletalMeshComponents;

    ID3D11VertexShader* VertexShader;
    ID3D11InputLayout* InputLayout;
    
    ID3D11PixelShader* PixelShader;
    ID3D11PixelShader* DebugDepthShader;
    ID3D11PixelShader* DebugWorldNormalShader;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
    
    FShadowManager* ShadowManager;
    FLoaderFBX* FBXLoader;
};
