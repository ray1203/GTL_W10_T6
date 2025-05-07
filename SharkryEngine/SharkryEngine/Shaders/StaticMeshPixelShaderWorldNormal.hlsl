
#include "ShaderRegisters.hlsl"

SamplerState NormalSampler : register(s1);

Texture2D NormalTexture : register(t1);

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = normalize(mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld));
    }
    
    float4 FinalColor = float4(WorldNormal, 1.f);
    
    FinalColor = (FinalColor + 1.f) / 2.f;
    
    return FinalColor;
}
