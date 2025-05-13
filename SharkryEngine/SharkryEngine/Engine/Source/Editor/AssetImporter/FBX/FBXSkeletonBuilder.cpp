// FBXSkeletonBuilder.cpp

#include "FBXSkeletonBuilder.h"
#include "FBXConversionUtil.h"
#include <fbxsdk.h>

#include "Animation/Skeleton.h"

void FBX::FBXSkeletonBuilder::CollectBoneNodes(FbxScene* Scene, TArray<FbxNode*>& OutBoneNodes)
{
    OutBoneNodes.Empty();
    for (int meshIdx = 0; meshIdx < Scene->GetSrcObjectCount<FbxMesh>(); ++meshIdx)
    {
        FbxMesh* Mesh = Scene->GetSrcObject<FbxMesh>(meshIdx);
        if (!Mesh) continue;

        int DeformerCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int deformerIdx = 0; deformerIdx < DeformerCount; ++deformerIdx)
        {
            FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin));
            if (!Skin) continue;

            int ClusterCount = Skin->GetClusterCount();
            for (int clusterIdx = 0; clusterIdx < ClusterCount; ++clusterIdx)
            {
                FbxCluster* Cluster = Skin->GetCluster(clusterIdx);
                if (Cluster && Cluster->GetLink())
                {
                    OutBoneNodes.AddUnique(Cluster->GetLink());
                }
            }
        }
    }
}

bool FBX::FBXSkeletonBuilder::EvaluateBoneBindPose(FbxNode* BoneNode, const FbxScene* Scene, FBoneHierarchyNode& OutNode)
{
    FbxAMatrix GlobalBindPoseMatrix;
    FbxAMatrix TransformMatrix;
    bool bBindPoseFound = false;

    for (int meshIdx = 0; meshIdx < Scene->GetSrcObjectCount<FbxMesh>() && !bBindPoseFound; ++meshIdx)
    {
        FbxMesh* Mesh = Scene->GetSrcObject<FbxMesh>(meshIdx);
        if (!Mesh) continue;

        int DeformerCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int deformerIdx = 0; deformerIdx < DeformerCount && !bBindPoseFound; ++deformerIdx)
        {
            FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin));
            if (!Skin) continue;

            int ClusterCount = Skin->GetClusterCount();
            for (int clusterIdx = 0; clusterIdx < ClusterCount && !bBindPoseFound; ++clusterIdx)
            {
                FbxCluster* Cluster = Skin->GetCluster(clusterIdx);
                if (Cluster && Cluster->GetLink() == BoneNode)
                {
                    Cluster->GetTransformLinkMatrix(GlobalBindPoseMatrix);
                    Cluster->GetTransformMatrix(TransformMatrix);
                    OutNode.GlobalBindPose = ConvertFbxAMatrixToFMatrix(GlobalBindPoseMatrix);
                    OutNode.TransformMatrix = ConvertFbxAMatrixToFMatrix(TransformMatrix);
                    bBindPoseFound = true;
                }
            }
        }
    }

    if (!bBindPoseFound)
    {
        GlobalBindPoseMatrix = BoneNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
        OutNode.GlobalBindPose = ConvertFbxAMatrixToFMatrix(GlobalBindPoseMatrix);
        OutNode.TransformMatrix = FMatrix::Identity;
    }

    return true;
}

void FBX::FBXSkeletonBuilder::ResolveParentChain(
    const TArray<FbxNode*>& BoneNodes,
    const TMap<FbxNode*, FName>& BoneNodeToNameMap,
    TMap<FName, FBoneHierarchyNode>& InOutHierarchy,
    TArray<FName>& OutRootBoneNames)
{
    for (FbxNode* BoneNode : BoneNodes)
    {
        FName BoneName = BoneNodeToNameMap[BoneNode];

        FbxNode* ParentNode = BoneNode->GetParent();
        FName ParentName = NAME_None;
        while (ParentNode)
        {
            const FName* FoundParentName = BoneNodeToNameMap.Find(ParentNode);
            if (FoundParentName && InOutHierarchy.Contains(*FoundParentName))
            {
                ParentName = *FoundParentName;
                break;
            }
            ParentNode = ParentNode->GetParent();
        }

        InOutHierarchy[BoneName].ParentName = ParentName;
        if (ParentName.IsNone())
        {
            OutRootBoneNames.AddUnique(BoneName);
        }
    }
}

void FBX::FBXSkeletonBuilder::BuildSkeletonHierarchy(
    FbxScene* Scene,
    TMap<FName, FBoneHierarchyNode>& OutSkeletonHierarchy,
    TArray<FName>& OutRootBoneNames)
{
    OutSkeletonHierarchy.Empty();
    OutRootBoneNames.Empty();

    TArray<FbxNode*> BoneNodes;
    TMap<FbxNode*, FName> BoneNodeToNameMap;

    CollectBoneNodes(Scene, BoneNodes);

    for (FbxNode* BoneNode : BoneNodes)
    {
        FName BoneName(BoneNode->GetName());
        BoneNodeToNameMap.Add(BoneNode, BoneName);

        if (!OutSkeletonHierarchy.Contains(BoneName))
        {
            FBoneHierarchyNode HierarchyNode;
            HierarchyNode.BoneName = BoneName;
            EvaluateBoneBindPose(BoneNode, Scene, HierarchyNode);
            OutSkeletonHierarchy.Add(BoneName, HierarchyNode);
        }
    }

    ResolveParentChain(BoneNodes, BoneNodeToNameMap, OutSkeletonHierarchy, OutRootBoneNames);
}
void FBX::FBXSkeletonBuilder::CalculateInitialLocalTransformsInternal(USkeleton* OutSkeleton)
{
    if (OutSkeleton->BoneTree.IsEmpty()) return;

    OutSkeleton->CurrentPose.Resize(OutSkeleton->BoneTree.Num());

    TArray<int32> ProcessingOrder; ProcessingOrder.Reserve(OutSkeleton->BoneTree.Num());
    TArray<uint8> Processed;
    Processed.Init(false, OutSkeleton->BoneTree.Num());
    TArray<int32> Queue; Queue.Reserve(OutSkeleton->BoneTree.Num());

    for (int32 i = 0; i < OutSkeleton->BoneTree.Num(); ++i)
    {
        if (OutSkeleton->BoneTree[i].ParentIndex == INDEX_NONE)
        {
            Queue.Add(i);
        }
    }

    int32 Head = 0;
    while (Head < Queue.Num())
    {
        int32 CurrentIndex = Queue[Head++];
        if (Processed[CurrentIndex])
        {
            continue;
        }
        ProcessingOrder.Add(CurrentIndex);
        Processed[CurrentIndex] = true;
        for (int32 i = 0; i < OutSkeleton->BoneTree.Num(); ++i)
        {
            if (OutSkeleton->BoneTree[i].ParentIndex == CurrentIndex && !Processed[i])
            {
                Queue.Add(i);
            }
        }

    }
    if (ProcessingOrder.Num() != OutSkeleton->BoneTree.Num())
    {
        for (int32 i = 0; i < OutSkeleton->BoneTree.Num(); ++i)
        {
            if (!Processed[i])
            {
                ProcessingOrder.Add(i);
            }
        }
    }

    for (int32 BoneIndex : ProcessingOrder)
    {
        const FBoneNode& CurrentBone = OutSkeleton->BoneTree[BoneIndex];
        const FMatrix& LocalBindPose = CurrentBone.BindTransform;
        int32 ParentIdx = CurrentBone.ParentIndex;

        // 1. 현재 포즈의 로컬 변환을 로컬 바인드 포즈로 초기화 (모든 본에 대해 수행)
        OutSkeleton->CurrentPose.LocalTransforms[BoneIndex] = LocalBindPose;

        // 2. 현재 포즈의 글로벌 변환 계산
        if (ParentIdx != INDEX_NONE) // 자식 본인 경우
        {
            if (OutSkeleton->CurrentPose.GlobalTransforms.IsValidIndex(ParentIdx))
            {
                const FMatrix& ParentGlobalTransform = OutSkeleton->CurrentPose.GlobalTransforms[ParentIdx];
                OutSkeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalBindPose * ParentGlobalTransform;
            }
            else
            {
                OutSkeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalBindPose; // 오류상황 : 임시 처리
            }
        }
        else // 루트 본인 경우
        {
            // 루트 글로벌 = 루트 로컬
            OutSkeleton->CurrentPose.GlobalTransforms[BoneIndex] = LocalBindPose;
        }

        // 3. 현재 포즈의 스키닝 행렬 계산 (모든 본에 대해 루프 끝에서 한 번만)
        OutSkeleton->CurrentPose.SkinningMatrices[BoneIndex] =
            OutSkeleton->CalculateSkinningMatrix(BoneIndex, OutSkeleton->CurrentPose.GlobalTransforms[BoneIndex]);
    }
}
