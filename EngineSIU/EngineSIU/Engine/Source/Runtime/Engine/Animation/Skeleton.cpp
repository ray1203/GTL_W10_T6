#include "Skeleton.h"

void FAnimationPoseData::Resize(int32 NumBones)
{
    LocalTransforms.SetNum(NumBones);
    GlobalTransforms.SetNum(NumBones);
    SkinningMatrices.SetNum(NumBones);
}

USkeleton::USkeleton()
{
}

void USkeleton::AddBone(const FName Name, int32 ParentIdx, const FMatrix BindTransform)
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
    // 현재 바로 GlobalBindTransform을 넣어주고 있어 주석, 로컬로 받도록 하려면 활성화
    //FMatrix GlobalBindTransform = CalculateGlobalBindTransform(BoneTree.Num() - 1);
    ReferenceSkeleton.RefBonePose.Add(BindTransform);

    // 현재 포즈 크기 조정
    CurrentPose.Resize(BoneTree.Num());
}

void USkeleton::AddBone(const FName Name, const FName ParentName, const FMatrix BindTransform)
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

uint32 USkeleton::GetBoneIndex(const FName Name) const
{
    const uint32* it = BoneNameToIndex.Find(Name);
    return (it != nullptr) ? *it : INDEX_NONE;
}

FMatrix USkeleton::GetGlobalBindTransform(int32 BoneIdx) const
{
    if (ReferenceSkeleton.RefBonePose.IsValidIndex(BoneIdx))
    {
        return ReferenceSkeleton.RefBonePose[BoneIdx];
    }

    return CalculateGlobalBindTransform(BoneIdx);
}

FMatrix USkeleton::CalculateGlobalBindTransform(int32 BoneIdx) const
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

FMatrix USkeleton::GetInverseBindTransform(int32 BoneIdx) const
{
    return FMatrix::Inverse(GetGlobalBindTransform(BoneIdx));
}

FMatrix USkeleton::CalculateSkinningMatrix(int32 BoneIdx, const FMatrix& AnimationMatrix) const
{
    return GetInverseBindTransform(BoneIdx) * AnimationMatrix;
}

void USkeleton::UpdateCurrentPose(const TArray<FMatrix>& LocalAnimationTransforms)
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
