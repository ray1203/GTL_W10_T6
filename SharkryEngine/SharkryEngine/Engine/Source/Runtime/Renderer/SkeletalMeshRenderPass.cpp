#include "SkeletalMeshRenderPass.h"

#include <array>

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"
#include "ShadowManager.h"
#include "ShadowRenderPass.h"
#include "ShowFlag.h"
#include "UnrealClient.h"
#include "Math/JungleMath.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/SkeletalMeshComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "UnrealEd/EditorViewportClient.h"
#include "Components/Light/PointLightComponent.h"

#include "FLoaderFBX.h"
#include "FSkeletalMeshDebugger.h"

FSkeletalMeshRenderPass::FSkeletalMeshRenderPass()
    : VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FSkeletalMeshRenderPass ::~FSkeletalMeshRenderPass()
{
    ReleaseShader();
}

void FSkeletalMeshRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC SkeletalMeshLayoutDesc[] = {
    { "POSITION",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",           0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",          0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TANGENT",         0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",        0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIAL_INDEX",  0, DXGI_FORMAT_R32_UINT,           0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEINDEX",       0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEWEIGHT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Begin Debug Shaders
    HRESULT hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShaderDepth", L"Shaders/StaticMeshPixelShaderDepth.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
    hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShaderWorldNormal", L"Shaders/StaticMeshPixelShaderWorldNormal.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
    // End Debug Shaders
#pragma region SkeletalMeshVertexShaders

/*
// Gouraud + GPU
    D3D_SHADER_MACRO DefinesGouraudGpu[] = {
        { "LIGHTING_MODEL_GOURAUD", "1" },
        { "GPU_SKINNING", "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"GOURAUD_SkeletalMeshVertexShader_GPU",
        L"Shaders/SkeletalMeshVertexShader.hlsl",
        "mainVS",
        SkeletalMeshLayoutDesc,
        ARRAYSIZE(SkeletalMeshLayoutDesc),
        DefinesGouraudGpu);
    if (FAILED(hr)) return;

    // Gouraud + CPU
    D3D_SHADER_MACRO DefinesGouraudCpu[] = {
        { "LIGHTING_MODEL_GOURAUD", "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"GOURAUD_SkeletalMeshVertexShader_CPU",
        L"Shaders/SkeletalMeshVertexShader.hlsl",
        "mainVS",
        SkeletalMeshLayoutDesc,
        ARRAYSIZE(SkeletalMeshLayoutDesc),
        DefinesGouraudCpu);
    if (FAILED(hr)) return;
    */

    // Default (non-Gouraud) + GPU
    D3D_SHADER_MACRO DefinesDefaultGpu[] = {
        { "GPU_SKINNING", "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"SkeletalMeshVertexShader_GPU",
        L"Shaders/SkeletalMeshVertexShader.hlsl",
        "mainVS",
        SkeletalMeshLayoutDesc,
        ARRAYSIZE(SkeletalMeshLayoutDesc),
        DefinesDefaultGpu);
    if (FAILED(hr)) return;

    // Default (non-Gouraud) + CPU
    hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"SkeletalMeshVertexShader_CPU",
        L"Shaders/SkeletalMeshVertexShader.hlsl",
        "mainVS",
        SkeletalMeshLayoutDesc,
        ARRAYSIZE(SkeletalMeshLayoutDesc));
    if (FAILED(hr)) return;

#pragma endregion SkeletalMeshVertexShaders

#pragma region UberShader
    D3D_SHADER_MACRO DefinesGouraud[] =
    {
        { GOURAUD, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"GOURAUD_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesGouraud);
    if (FAILED(hr))
    {
        return;
    }

    D3D_SHADER_MACRO DefinesLambert[] =
    {
        { LAMBERT, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"LAMBERT_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesLambert);
    if (FAILED(hr))
    {
        return;
    }

    D3D_SHADER_MACRO DefinesBlinnPhong[] =
    {
        { PHONG, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"PHONG_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesBlinnPhong);
    if (FAILED(hr))
    {
        return;
    }

#pragma endregion UberShader

    VertexShader = ShaderManager->GetVertexShaderByKey(L"SkeletalMeshVertexShader_GPU");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"SkeletalMeshVertexShader_GPU");

    PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
    DebugDepthShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderDepth");
    DebugWorldNormalShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderWorldNormal");
}

void FSkeletalMeshRenderPass::ReleaseShader()
{

}

void FSkeletalMeshRenderPass::ChangeViewMode(EViewModeIndex ViewModeIndex)
{
    switch (ViewModeIndex)
    {
    case EViewModeIndex::VMI_Lit_Gouraud:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"GOURAUD_StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"GOURAUD_StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"GOURAUD_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;

    case EViewModeIndex::VMI_Lit_Lambert:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"LAMBERT_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;

    case EViewModeIndex::VMI_Lit_BlinnPhong:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;

    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
        UpdateLitUnlitConstant(0);
        break;

    case EViewModeIndex::VMI_SceneDepth:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderDepth");
        UpdateLitUnlitConstant(0);
        break;

    case EViewModeIndex::VMI_WorldNormal:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderWorldNormal");
        UpdateLitUnlitConstant(0);
        break;

    default:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;
    }
}


void FSkeletalMeshRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
}

void FSkeletalMeshRenderPass::InitializeShadowManager(class FShadowManager* InShadowManager)
{
    ShadowManager = InShadowManager;
}

void FSkeletalMeshRenderPass::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<USkeletalMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            SkeletalMeshComponents.Add(iter);
        }
    }
}

void FSkeletalMeshRenderPass::PrepareRenderState(const std::shared_ptr<FViewportClient>& Viewport)
{
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    ChangeViewMode(ViewMode);

    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    TArray<FString> PSBufferKeys = {
        TEXT("FLightInfoBuffer"),
        TEXT("FMaterialConstants"),
        TEXT("FLitUnlitConstants"),
        TEXT("FSubMeshConstants"),
        TEXT("FTextureConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    BufferManager->BindConstantBuffer(TEXT("FLightInfoBuffer"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FMaterialConstants"), 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("BonesBuffer"), 2, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FObjectConstantBuffer"), 12, EShaderStage::Vertex);


    Graphics->DeviceContext->RSSetViewports(1, &Viewport->GetViewportResource()->GetD3DViewport());

    const EResourceType ResourceType = EResourceType::ERT_Scene;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    FDepthStencilRHI* DepthStencilRHI = ViewportResource->GetDepthStencil(ResourceType);

    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, DepthStencilRHI->DSV);


    // Rasterizer
    if (ViewMode == EViewModeIndex::VMI_Wireframe)
    {
        Graphics->DeviceContext->RSSetState(Graphics->RasterizerWireframeBack);
    }
    else
    {
        Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
    }

    // Pixel Shader
    if (ViewMode == EViewModeIndex::VMI_SceneDepth)
    {
        Graphics->DeviceContext->PSSetShader(DebugDepthShader, nullptr, 0);
    }
    else if (ViewMode == EViewModeIndex::VMI_WorldNormal)
    {
        Graphics->DeviceContext->PSSetShader(DebugWorldNormalShader, nullptr, 0);
    }
    else
    {
        Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    }
}

void FSkeletalMeshRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;

    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}
void FSkeletalMeshRenderPass::UpdateBoneBuffer(const TArray<FMatrix>& SkinningMatrices) const
{
    FBoneMatrixBuffer BufferData = {};

    const int32 CopyCount = FMath::Min(SkinningMatrices.Num(), 128);
    for (int32 i = 0; i < CopyCount; ++i)
    {
        BufferData.BoneMatrices[i] = SkinningMatrices[i];
    }

    BufferManager->UpdateConstantBuffer(TEXT("BonesBuffer"), BufferData);
}

void FSkeletalMeshRenderPass::UpdateLitUnlitConstant(int32 isLit) const
{
    FLitUnlitConstants Data;
    Data.bIsLit = isLit;
    BufferManager->UpdateConstantBuffer(TEXT("FLitUnlitConstants"), Data);
}

void FSkeletalMeshRenderPass::RenderPrimitive(
    ID3D11Buffer* VertexBuffer,
    FBX::FSkeletalMeshRenderData* RenderData,
    TArray<FStaticMaterial*> Materials,
    TArray<UMaterial*> OverrideMaterials,
    int SelectedSubMeshIndex) const {
    // 정점 스트라이드 변경: FStaticMeshVertex -> FBX::FSkeletalMeshVertex
    UINT Stride = sizeof(FBX::FSkeletalMeshVertex);
    UINT Offset = 0;

    // 버퍼 설정 (DynamicVertexBuffer 사용)
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (RenderData->Subsets.IsEmpty())
    {
        if (!RenderData->Indices.IsEmpty())
        {
            // 기본 재질 (예: 첫 번째 재질) 설정 시도 
            UMaterial* MaterialToUse = nullptr;
            if (OverrideMaterials.IsValidIndex(0) && OverrideMaterials[0] != nullptr)
            {
                MaterialToUse = OverrideMaterials[0];
            }
            else if (Materials.IsValidIndex(0) && Materials[0] != nullptr && Materials[0]->Material != nullptr)
            {
                MaterialToUse = Materials[0]->Material;
            }

            if (MaterialToUse)
            {
                // UpdateMaterial은 FObjMaterialInfo를 받음
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, MaterialToUse->GetMaterialInfo());
            }
            else
            {
                // TODO: 기본 재질 처리 (예: 회색 재질)
            }
            // 전체 인덱스 그리기
            Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        }
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->Subsets.Num(); ++SubMeshIndex)
    {
        const FBX::FMeshSubset& Subset = RenderData->Subsets[SubMeshIndex];
        uint32 MaterialIndex = Subset.MaterialIndex;

        FSubMeshConstants SubMeshData;
        SubMeshData.bIsSelectedSubMesh = (SubMeshIndex == SelectedSubMeshIndex) ? 1.0f : 0.0f;
        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        UMaterial* MaterialToUse = nullptr;
        if (OverrideMaterials.IsValidIndex(MaterialIndex) && OverrideMaterials[MaterialIndex])
        {
            MaterialToUse = OverrideMaterials[MaterialIndex];
        }
        else if (Materials.IsValidIndex(MaterialIndex) && Materials[MaterialIndex] && Materials[MaterialIndex]->Material)
        {
            MaterialToUse = Materials[MaterialIndex]->Material;
        }

        if (MaterialToUse)
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, MaterialToUse->GetMaterialInfo());
        }

        if (Subset.IndexCount > 0)
        {
            Graphics->DeviceContext->DrawIndexed(Subset.IndexCount, Subset.IndexStart, 0);
        }
    }
}

void FSkeletalMeshRenderPass::RenderAllSkeletalMeshes(const std::shared_ptr<FViewportClient>& Viewport)
{
    const uint64 ShowFlag = Viewport->GetShowFlag();

    for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
    {
        if (!Comp || !Comp->GetSkeletalMesh()) continue;

        USkeletalMesh* SkeletalMesh = Comp->GetSkeletalMesh();
        FBX::FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetRenderData();
        FBX::FSkeletalMeshInstanceRenderData* InstanceData = Comp->GetInstanceRenderData();
        if (!RenderData || !InstanceData) continue;

        const bool bGpu = Comp->IsUsingGpuSkinning();
        const EViewModeIndex ViewMode = Viewport->GetViewMode();
        const bool bGouraud = ViewMode == EViewModeIndex::VMI_Lit_Gouraud;

        if (ShowFlag & EEngineShowFlags::SF_Bone)
        {
            FSkeletalMeshDebugger::DrawSkeleton(Comp);
            FSkeletalMeshDebugger::DrawSkeletonAABBs(Comp);
        }

        FWString ShaderKey;
        if (bGpu)
            ShaderKey = bGouraud ? L"GOURAUD_SkeletalMeshVertexShader_GPU" : L"SkeletalMeshVertexShader_GPU";
        else
            ShaderKey = bGouraud ? L"GOURAUD_SkeletalMeshVertexShader_CPU" : L"SkeletalMeshVertexShader_CPU";

        ID3D11VertexShader* CompVS = ShaderManager->GetVertexShaderByKey(ShaderKey);
        ID3D11InputLayout* CompIL = ShaderManager->GetInputLayoutByKey(ShaderKey);

        Graphics->DeviceContext->VSSetShader(CompVS, nullptr, 0);
        Graphics->DeviceContext->IASetInputLayout(CompIL);

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
        AActor* SelectedActor = Engine->GetSelectedActor();
        USceneComponent* TargetComponent = SelectedComponent ? SelectedComponent : SelectedActor ? SelectedActor->GetRootComponent() : nullptr;

        const bool bIsSelected = (Engine && TargetComponent == Comp);
        const FMatrix WorldMatrix = Comp->GetWorldMatrix();
        const FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;

        UpdateObjectConstant(WorldMatrix, UUIDColor, bIsSelected);

        if (bGpu)
        {
            UpdateBoneBuffer(SkeletalMesh->Skeleton->CurrentPose.SkinningMatrices);
            RenderPrimitive(RenderData->SharedVertexBuffer, RenderData, SkeletalMesh->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());
        }
        else
        {
            RenderPrimitive(InstanceData->DynamicVertexBuffer_CPU, RenderData, SkeletalMesh->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());
        }

        if (ShowFlag & EEngineShowFlags::SF_AABB)
        {
            FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(Comp->GetBoundingBox(), WorldMatrix);
        }
    }
}


void FSkeletalMeshRenderPass::Render(const std::shared_ptr<FViewportClient>& Viewport)
{
    //if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
    //{
    //    ShadowRenderPass->RenderCubeMap(Viewport, PointLight);
    //    RenderAllStaticMeshesForPointLight(Viewport, PointLight);
    //}

    ShadowManager->BindResourcesForSampling();

    PrepareRenderState(Viewport);

    RenderAllSkeletalMeshes(Viewport);

    // 렌더 타겟 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_PointLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_DirectionalLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_SpotLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정


    // @todo 리소스 언바인딩 필요한가?
    // SRV 해제
    ID3D11ShaderResourceView* NullSRVs[14] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 14, NullSRVs);

    // 상수버퍼 해제
    ID3D11Buffer* NullPSBuffer[9] = { nullptr };
    Graphics->DeviceContext->PSSetConstantBuffers(0, 9, NullPSBuffer);
    ID3D11Buffer* NullVSBuffer[2] = { nullptr };
    Graphics->DeviceContext->VSSetConstantBuffers(0, 2, NullVSBuffer);

}

void FSkeletalMeshRenderPass::ClearRenderArr()
{
    SkeletalMeshComponents.Empty();
}


/*
void FSkeletalMeshRenderPass::RenderAllSkeletalMeshes(const std::shared_ptr<FViewportClient>&Viewport, UPointLightComponent * &PointLight)
{
    for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
    {
        if (!Comp || !Comp->GetSkeletalMesh()) { continue; }

        FBX::FSkeletalMeshRenderData * RenderData = Comp->GetSkeletalMesh()->GetRenderData();
        if (RenderData == nullptr) { continue; }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        FMatrix WorldMatrix = Comp->GetWorldMatrix();

        //ShadowRenderPass->UpdateCubeMapConstantBuffer(PointLight, WorldMatrix);

        RenderPrimitive(RenderData, Comp->GetSkeletalMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());
    }
}
*/
