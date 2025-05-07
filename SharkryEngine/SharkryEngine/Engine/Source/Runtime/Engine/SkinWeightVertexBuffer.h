#pragma once
#include "Core/HAL/PlatformType.h"
#include "Core/Container/Array.h"

struct FSkinWeightInfo
{
    uint8 BoneIndices[4];
    uint8 BoneWeights[4];
};

class FSkinWeightVertexBuffer
{
public:
    FSkinWeightVertexBuffer();

    void Init(uint32 InNumVertices);

    void GetSkinWeights(TArray<FSkinWeightInfo>& OutSkinWeights) const;

    void GetSkinWeights(FSkinWeightInfo& OutWeight, uint32 VertexIndex) const;

    // 특정 버텍스의 본 인덱스 가져오기
    int32 GetBoneIndex(uint32 VertexIndex, uint32 InfluenceIndex) const;

    // 특정 버텍스의 가중치 가져오기
    float GetBoneWeight(uint32 VertexIndex, uint32 InfluenceIndex) const;

    // 특정 버텍스의 스킨 가중치 정보 설정
    void SetVertexSkinWeights(uint32 VertexIndex, const FSkinWeightInfo& InWeights);

    // 버텍스 수 반환
    uint32 GetNumVertices() const;

private:
    uint32 NumVertices;
    TArray<FSkinWeightInfo> SkinWeights;
};
