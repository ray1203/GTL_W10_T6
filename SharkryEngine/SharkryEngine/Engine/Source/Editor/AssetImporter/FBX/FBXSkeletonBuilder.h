#pragma once

#include "Define.h"
#include "FBXStructs.h"
#include <fbxsdk.h>

class USkeleton;

namespace FBX
{
    /// FBX 본 계층을 추출하여 SkeletonHierarchy 구조로 구성하는 유틸리티
    class FBXSkeletonBuilder
    {
    public:
        /// FBX 씬에서 본 계층을 추출하여 SkeletonHierarchy 및 RootBoneNames에 기록
        static void BuildSkeletonHierarchy(
            FbxScene* Scene,
            TMap<FName, FBoneHierarchyNode>& OutSkeletonHierarchy,
            TArray<FName>& OutRootBoneNames
        );

        /// 클러스터를 순회하여 모든 본 노드를 추출
        static void CollectBoneNodes(FbxScene* Scene, TArray<FbxNode*>& OutBoneNodes);

        /// 특정 BoneNode에서 BindPose 및 TransformMatrix 계산
        static bool EvaluateBoneBindPose(
            FbxNode* BoneNode,
            const FbxScene* Scene,
            FBoneHierarchyNode& OutNode
        );

        /// FBX 본 이름 → Parent 이름을 연결
        static void ResolveParentChain(
            const TArray<FbxNode*>& BoneNodes,
            const TMap<FbxNode*, FName>& BoneNodeToNameMap,
            TMap<FName, FBoneHierarchyNode>& InOutHierarchy,
            TArray<FName>& OutRootBoneNames
        );
        static void CalculateInitialLocalTransformsInternal(USkeleton* OutSkeleton);

    };
}
