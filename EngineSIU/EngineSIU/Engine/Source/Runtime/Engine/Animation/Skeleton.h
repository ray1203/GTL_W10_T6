#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

struct FBoneNode
{
    FName Name;
    int32 ParentIndex;
    FMatrix BindTransform; // Bind pose transform (로컬 바인드 포즈)

    // 리타겟팅 모드 - Animation 주차에 필요 시 사용
    // TEnumAsByte<EBoneTranslationRetargetingMode::Type> TranslationRetargetingMode;

    FBoneNode()
        : ParentIndex(INDEX_NONE)
        , BindTransform(FMatrix::Identity)
        // , TranslationRetargetingMode(EBoneTranslationRetargetingMode::Animation)
    {
    }
};

struct FReferenceSkeleton
{
    TArray<FBoneNode> BoneInfo;

    // 각 본의 글로벌 바인드 포즈
    TArray<FMatrix> RefBonePose;

    TMap<FName, int32> NameToIndexMap;
};

struct FSkeletonToMeshLinkup
{
    // Skeleton Bone Index → Mesh Bone Index
    TArray<int32> SkeletonToMeshTable;

    // Mesh Bone Index → Skeleton Bone Index
    TArray<int32> MeshToSkeletonTable;
};

// Pose data of current animation
struct FAnimationPoseData
{
    // 각 본의 로컬 변환 행렬 (애니메이션 적용 후)
    TArray<FMatrix> LocalTransforms;

    // 각 본의 글로벌 변환 행렬 (애니메이션 적용 후)
    TArray<FMatrix> GlobalTransforms;

    // 스키닝 행렬 (애니메이션 행렬 * 인버스 바인드 포즈 행렬)
    TArray<FMatrix> SkinningMatrices;

    void Resize(int32 NumBones)
    {
        LocalTransforms.SetNum(NumBones);
        GlobalTransforms.SetNum(NumBones);
        SkinningMatrices.SetNum(NumBones);
    }
};

class USkeleton : public UObject
{
    DECLARE_CLASS(USkeleton, UObject)
public:
    TArray<FBoneNode> BoneTree;

    TMap<FName, FName> BoneParentMap;

    // 본 이름에서 인덱스로의 맵
    TMap<FName, uint32> BoneNameToIndex;

    // 참조 스켈레톤
    FReferenceSkeleton ReferenceSkeleton;

    // Mesh - Skeleton index cache 나중에 필요 시 사용
    TArray<FSkeletonToMeshLinkup> LinkupCache;

    // 현재 애니메이션 포즈 데이터 - Unreal기준 USkeleton에 없으나 임시 사용
    FAnimationPoseData CurrentPose;

public:
    USkeleton();
    ~USkeleton() = default;

    // 본 추가
    void AddBone(const FName Name, int32 ParentIdx, const FMatrix BindTransform)
    {
        BoneNameToIndex[Name] = (int)BoneTree.Num();

        FBoneNode NewBone;
        NewBone.Name = Name;
        NewBone.ParentIndex = ParentIdx;
        NewBone.BindTransform = BindTransform;

        BoneTree.Add(NewBone);

        // 참조 스켈레톤 업데이트
        ReferenceSkeleton.BoneInfo.Add(NewBone);
        ReferenceSkeleton.NameToIndexMap.Add(Name, ReferenceSkeleton.BoneInfo.Num() - 1);

        // 글로벌 바인드 포즈 계산 및 추가
        //FMatrix GlobalBindTransform = CalculateGlobalBindTransform(BoneTree.Num() - 1);
        ReferenceSkeleton.RefBonePose.Add(BindTransform);

        // 현재 포즈 크기 조정
        CurrentPose.Resize(BoneTree.Num());
    }

    void AddBone(const FName Name, const FName ParentName, const FMatrix BindTransform)
    {
        int32 ParentIdx = INDEX_NONE;

        // 부모 이름이 유효하면 해당 인덱스 찾기
        if (!ParentName.IsNone())
        {
            const uint32* ParentIdxPtr = BoneNameToIndex.Find(ParentName);
            if (ParentIdxPtr)
            {
                ParentIdx = static_cast<int32>(*ParentIdxPtr);
            }
        }

        // 부모 관계 맵에 저장
        BoneParentMap.Add(Name, ParentName);

        // 기존 AddBone 호출
        AddBone(Name, ParentIdx, BindTransform);
    }

    // 본 이름으로 인덱스 가져오기
    uint32 GetBoneIndex(const FName Name) const
    {
        const uint32* it = BoneNameToIndex.Find(Name);
        return (it != nullptr) ? *it : INDEX_NONE;
    }

    // 글로벌 바인드 트랜스폼 계산
    FMatrix GetGlobalBindTransform(int32 BoneIdx) const
    {
        if (ReferenceSkeleton.RefBonePose.IsValidIndex(BoneIdx))
        {
            return ReferenceSkeleton.RefBonePose[BoneIdx];
        }

        return CalculateGlobalBindTransform(BoneIdx);
    }

    // 글로벌 바인드 트랜스폼 계산 (내부 함수)
    FMatrix CalculateGlobalBindTransform(int32 BoneIdx) const
    {
        FMatrix Global = BoneTree[BoneIdx].BindTransform;
        int32 Parent = BoneTree[BoneIdx].ParentIndex;
        while (Parent >= 0)
        {
            Global = BoneTree[Parent].BindTransform * Global;
            Parent = BoneTree[Parent].ParentIndex;
        }

        return Global;
    }

    // 인버스 바인드 포즈 행렬 가져오기
    FMatrix GetInverseBindTransform(int32 BoneIdx) const
    {
        return FMatrix::Inverse(GetGlobalBindTransform(BoneIdx));
    }

    // 스키닝 행렬 계산 (애니메이션 행렬 * 인버스 바인드 포즈 행렬)
    FMatrix CalculateSkinningMatrix(int32 BoneIdx, const FMatrix& AnimationMatrix) const
    {
        return  GetInverseBindTransform(BoneIdx) * AnimationMatrix;
    }

    // 현재 애니메이션 포즈 업데이트
    void UpdateCurrentPose(const TArray<FMatrix>& LocalAnimationTransforms)
    {
        // 로컬 변환 저장
        for (int32 i = 0; i < FMath::Min(LocalAnimationTransforms.Num(), CurrentPose.LocalTransforms.Num()); ++i)
        {
            CurrentPose.LocalTransforms[i] = LocalAnimationTransforms[i];
        }

        // 글로벌 변환 계산
        for (int32 i = 0; i < CurrentPose.LocalTransforms.Num(); ++i)
        {
            if (BoneTree[i].ParentIndex == INDEX_NONE)
            {
                CurrentPose.GlobalTransforms[i] = CurrentPose.LocalTransforms[i];
            }
            else
            {
                CurrentPose.GlobalTransforms[i] = CurrentPose.GlobalTransforms[BoneTree[i].ParentIndex] * CurrentPose.LocalTransforms[i];
            }

            // 스키닝 행렬 계산
            CurrentPose.SkinningMatrices[i] = CalculateSkinningMatrix(i, CurrentPose.GlobalTransforms[i]);
        }
    }

    // 메시-스켈레톤 링크업 데이터 찾기 또는 추가 
    /*const FSkeletonToMeshLinkup& FindOrAddMeshLinkupData(const UObject* InMesh)
    {
    }*/

    // 메시 본 인덱스 → 스켈레톤 본 인덱스 변환
    //int32 GetSkeletonBoneIndexFromMeshBoneIndex(const UObject* InMesh, int32 MeshBoneIndex)
    //{
    //    return MeshBoneIndex;
    //}

    //// 스켈레톤 본 인덱스 → 메시 본 인덱스 변환
    //int32 GetMeshBoneIndexFromSkeletonBoneIndex(const UObject* InMesh, int32 SkeletonBoneIndex)
    //{
    //    return SkeletonBoneIndex;
    //}
};
