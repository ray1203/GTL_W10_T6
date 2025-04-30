#include "FadeRenderPass.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Define.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include <wchar.h>
#include <UObject/UObjectIterator.h>
#include "Engine/Engine.h"
#include "World/World.h"
#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "ShowFlag.h"
#include "Engine/Camera/PlayerCameraManager.h"
#include "Engine/Classes/GameFramework/PlayerController.h"

FFadeRenderPass::FFadeRenderPass()
{


}

FFadeRenderPass::~FFadeRenderPass()
{
    ReleaseShader();
}

void FFadeRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    CreateShader();
    CreateBlendState();
}

void FFadeRenderPass::PrepareRenderArr()
{
    if (GEngine->ActiveWorld&&GEngine->ActiveWorld->WorldType != EWorldType::Editor) {
            FadeAlpha = GEngine->ActiveWorld->GetFirstPlayerController()->PlayerCameraManager->FadeAmount;
            FadeColor = GEngine->ActiveWorld->GetFirstPlayerController()->PlayerCameraManager->FadeColor;
    }
    else
    {
        FadeAlpha = 0;
        FadeColor = FLinearColor(0, 0, 0, 0);
    }
}

void FFadeRenderPass::Render(const std::shared_ptr<FViewportClient>& Viewport)
{

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_PP_Fog;
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetViewportResource()->GetRenderTarget(ResourceType);

    ViewportResource->ClearRenderTarget(Graphics->DeviceContext, ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);

    PrepareRenderState();

    UpdateFadeConstant();

    float blendFactor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->OMSetBlendState(BlendState, blendFactor, 0xffffffff);

 
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->Draw(3, 0);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);
}


void FFadeRenderPass::CreateShader()
{
    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShader(L"FadeVertexShader", L"Shaders/FadeEffect.hlsl", "mainVS");
    if (FAILED(hr))
    {
        return;
    }
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(L"FadePixelShader", L"Shaders/FadeEffect.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }

    // 생성된 셰이더와 입력 레이아웃 획득
    VertexShader = ShaderManager->GetVertexShaderByKey(L"FadeVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"FadePixelShader");
}

void FFadeRenderPass::ReleaseShader()
{
   
        if (VertexShader) { VertexShader->Release(); VertexShader = nullptr; }
        if (PixelShader) { PixelShader->Release(); PixelShader = nullptr; }
        if (BlendState) { BlendState->Release();  BlendState = nullptr; }
    

}

void FFadeRenderPass::CreateBlendState()
{
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = Graphics->Device->CreateBlendState(&blendDesc, &BlendState);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"AlphaBlendState 생성에 실패했습니다!", L"Error", MB_ICONERROR | MB_OK);
    }
}

void FFadeRenderPass::ClearRenderArr()
{

}

void FFadeRenderPass::ReloadShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"FadeVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"FadePixelShader");
}

void FFadeRenderPass::PrepareRenderState()
{
    // 셰이더 설정
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    TArray<FString> PSBufferKeys = {
        TEXT("FFadeConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);
}

void FFadeRenderPass::UpdateFadeConstant()
{
    FFadeConstants Constants = {
        Constants.FadeColor = FadeColor, Constants.FadeAlpha = FadeAlpha    
    };
    BufferManager->UpdateConstantBuffer(TEXT("FFadeConstants"), Constants);
}


