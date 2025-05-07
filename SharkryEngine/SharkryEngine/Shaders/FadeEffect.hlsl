// FadeEffect.hlsl

// 상수 버퍼 : b0 슬롯
cbuffer FadeEffectCB : register(b0)
{
    float4 fadeColor;
    float fadeAlpha; // 0.0 = 투명, 1.0 = 완전 페이드
    float3 padding;
     // 페이드 색상 (예: 검정 = 0,0,0)
};

struct VSOut
{
    float4 pos : SV_Position;
};

// 풀스크린 트라이앵글 정점 위치
static float2 QuadPositions[6] =
{
    float2(-1, 1), // Top Left
        float2(1, 1), // Top Right
        float2(-1, -1), // Bottom Left
        float2(1, 1), // Top Right
        float2(1, -1), // Bottom Right
        float2(-1, -1) // Bottom Left
};

static float2 UVs[6] =
{
    float2(0, 0), float2(1, 0), float2(0, 1),
        float2(1, 0), float2(1, 1), float2(0, 1)
};


VSOut mainVS(uint vertexID : SV_VertexID)
{
    VSOut output;
    output.pos = float4(QuadPositions[vertexID], UVs[vertexID]);
    return output;
}

float4 mainPS(VSOut IN) : SV_Target
{
    // fadeColor는 그대로, alpha만 블렌드 스테이트로 적용
    return float4(fadeColor.rgb, fadeAlpha);
}
