#include "ShaderRegisters.hlsl"
/////////////////////////////////////////////
//실행하기 전 껐는지 확인
///////////////////////////////////////////////
//#define LIGHTING_MODEL_GOURAUD 1
//#define GPU_SKINNING 1
#ifdef LIGHTING_MODEL_GOURAUD
SamplerState DiffuseSampler : register(s0);
Texture2D DiffuseTexture : register(t0);

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

#include "Light.hlsl"
#endif

#ifdef GPU_SKINNING
cbuffer BonesBuffer : register(b2)
{
    row_major float4x4 boneMatrices[512];
}
#endif

PS_INPUT_StaticMesh mainVS(VS_INPUT_SkeletalMesh Input)
{
    PS_INPUT_StaticMesh Output;

#ifdef GPU_SKINNING
    // TODO: GPU Skinning 적용 (Weight & BoneMatrix 조합)
    float4 SkinnedPos = float4(0, 0, 0, 0);
    float3 SkinnedNormal = float3(0, 0, 0);

    for (int i = 0; i < 4; ++i)
    {
        uint boneIndex = Input.BoneIndices[i];
        float weight = Input.BoneWeights[i];
        //Weight는 이미 정규화된 값이라 가정
        SkinnedPos += mul(float4(Input.Position, 1.0),boneMatrices[boneIndex]) * weight;

        SkinnedNormal += mul(Input.Normal,(float3x3) boneMatrices[boneIndex]) * weight;
        // TODO: boneMatrices[boneIndex] * Input.Position
        // TODO: accumulate SkinnedPos, SkinnedNormal
    }

    Output.Position = SkinnedPos;
    Output.WorldNormal = SkinnedNormal;
    /*if (SkinnedPos==float4(0,0,0,0)){
        Output.Position = float4(Input.Position, 1.0);
        Output.WorldNormal = Input.Normal;
    }*/
    
#else
    // CPU Skinning 또는 StaticMesh와 동일 처리
    Output.Position = float4(Input.Position, 1.0);
    Output.WorldNormal = Input.Normal;
#endif

    Output.Position = mul(Output.Position, WorldMatrix);
    Output.WorldPosition = Output.Position.xyz;
    Output.Position = mul(Output.Position, ViewMatrix);
    Output.Position = mul(Output.Position, ProjectionMatrix);

    Output.WorldViewPosition = float3(InvViewMatrix._41, InvViewMatrix._42, InvViewMatrix._43);

    float3 BiTangent = cross(Output.WorldNormal, Input.Tangent);
    matrix<float, 3, 3> TBN =
    {
        Input.Tangent.x, Input.Tangent.y, Input.Tangent.z,
        BiTangent.x, BiTangent.y, BiTangent.z,
        Output.WorldNormal.x, Output.WorldNormal.y, Output.WorldNormal.z
    };
    Output.TBN = TBN;

    Output.UV = Input.UV;
    Output.MaterialIndex = Input.MaterialIndex;

#ifdef LIGHTING_MODEL_GOURAUD
    float4 Diffuse = Lighting(Output.WorldPosition, Output.WorldNormal, ViewWorldLocation, float3(1,1,1));
    Output.Color = float4(Diffuse.rgb, 1.0);
#else
    Output.Color = Input.Color;
#endif

    return Output;
}
