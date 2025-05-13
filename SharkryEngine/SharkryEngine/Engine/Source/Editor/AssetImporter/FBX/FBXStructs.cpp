#include "FBXStructs.h"
void FBX::FSkeletalMeshRenderData::ReleaseBuffers()
{
    if (SharedVertexBuffer)
    {
        SharedVertexBuffer->Release();
        SharedVertexBuffer = nullptr;
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
        IndexBuffer = nullptr;
    }
}

namespace FBX {


}
namespace std
{
    size_t hash<FBX::FSkeletalMeshVertex>::operator()(const FBX::FSkeletalMeshVertex& Key) const noexcept
    {
        size_t seed = 0;
        // 이제 헤더에 정의된 hash_combine을 사용할 수 있습니다.
        hash_combine(seed, std::hash<float>()(Key.Position.X));
        hash_combine(seed, std::hash<float>()(Key.Position.Y));
        hash_combine(seed, std::hash<float>()(Key.Position.Z));
        hash_combine(seed, std::hash<float>()(Key.Normal.X));
        hash_combine(seed, std::hash<float>()(Key.Normal.Y));
        hash_combine(seed, std::hash<float>()(Key.Normal.Z));
        hash_combine(seed, std::hash<float>()(Key.TexCoord.X));
        hash_combine(seed, std::hash<float>()(Key.TexCoord.Y));
        // TODO: Add Tangent hashing if member exists
        std::hash<uint32> uint_hasher;
        std::hash<float> float_hasher;
        for (int i = 0; i < MAX_BONE_INFLUENCES; ++i)
        {
            hash_combine(seed, uint_hasher(Key.BoneIndices[i]));
            hash_combine(seed, float_hasher(Key.BoneWeights[i]));
        }
        return seed;
    }
}
