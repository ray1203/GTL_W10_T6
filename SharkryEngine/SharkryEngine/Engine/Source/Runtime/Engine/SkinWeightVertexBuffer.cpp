#include "SkinWeightVertexBuffer.h"

FSkinWeightVertexBuffer::FSkinWeightVertexBuffer()
    : NumVertices(0)
{
}

void FSkinWeightVertexBuffer::Init(uint32 InNumVertices)
{
    NumVertices = InNumVertices;
    SkinWeights.SetNum(NumVertices);
}

void FSkinWeightVertexBuffer::GetSkinWeights(TArray<FSkinWeightInfo>& OutSkinWeights) const
{
    OutSkinWeights = SkinWeights;
}

void FSkinWeightVertexBuffer::GetSkinWeights(FSkinWeightInfo& OutWeight, uint32 VertexIndex) const
{
    if (VertexIndex < NumVertices)
    {
        OutWeight = SkinWeights[VertexIndex];
    }
}
int32 FSkinWeightVertexBuffer::GetBoneIndex(uint32 VertexIndex, uint32 InfluenceIndex) const
{
    if (VertexIndex < NumVertices && InfluenceIndex < 4)
    {
        return SkinWeights[VertexIndex].BoneIndices[InfluenceIndex];
    }
    return 0;
}

float FSkinWeightVertexBuffer::GetBoneWeight(uint32 VertexIndex, uint32 InfluenceIndex) const
{
    if (VertexIndex < NumVertices && InfluenceIndex < 4)
    {
        return SkinWeights[VertexIndex].BoneWeights[InfluenceIndex] / 255.0f;
    }
    return 0.0f;
}

void FSkinWeightVertexBuffer::SetVertexSkinWeights(uint32 VertexIndex, const FSkinWeightInfo& InWeights)
{
    if (VertexIndex < NumVertices)
    {
        SkinWeights[VertexIndex] = InWeights;
    }
}

uint32 FSkinWeightVertexBuffer::GetNumVertices() const
{
    return NumVertices;
}
