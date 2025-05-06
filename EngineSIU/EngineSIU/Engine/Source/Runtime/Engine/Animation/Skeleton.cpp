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

void USkeleton::AddBone(const FName Name, int32 ParentIdx, const FMatrix& InGlobalBindPose, const FMatrix& InTransformMatrix) // 파라미터 이름 명시
{
    int32 CurrentBoneIndex = BoneTree.Num(); // 새로 추가될 본의 인덱스
    BoneNameToIndex.Add(Name, CurrentBoneIndex);

    FBoneNode NewBoneNode; // 지역 변수
    NewBoneNode.Name = Name;
    NewBoneNode.ParentIndex = ParentIdx;

    // 1. 로컬 바인드 포즈 계산 -> NewBoneNode.BindTransform 에 저장
    if (ParentIdx != INDEX_NONE)
    {
        // 부모의 글로벌 바인드 포즈는 RefBonePose에 이미 저장되어 있어야 함
        if (ReferenceSkeleton.RefBonePose.IsValidIndex(ParentIdx))
        {
            const FMatrix& ParentGlobalBindPose = ReferenceSkeleton.RefBonePose[ParentIdx];
            FMatrix InverseParentGlobalBindPose = FMatrix::Inverse(ParentGlobalBindPose);
          
            // 행 우선: ChildLocal = ChildGlobal * ParentGlobalInv
            NewBoneNode.BindTransform = InGlobalBindPose * InverseParentGlobalBindPose;
        }
        else
        {
            NewBoneNode.BindTransform = InGlobalBindPose; //오류 상황
        }
    }
    else
    {
        // 루트 본: 로컬 바인드 포즈 = 글로벌 바인드 포즈
        NewBoneNode.BindTransform = InGlobalBindPose;
    }

    // 2. 글로벌 바인드 포즈의 역행렬 계산 -> NewBoneNode.InverseBindTransform 에 저장
    NewBoneNode.InverseBindTransform = FMatrix::Inverse(InGlobalBindPose);
    NewBoneNode.GeometryOffsetMatrix = InTransformMatrix;
    if (FMath::IsNearlyZero(NewBoneNode.InverseBindTransform.Determinant())) 
    {
        NewBoneNode.InverseBindTransform = FMatrix::Identity; // 방어 코드
    }

    // 3. 완성된 NewBoneNode를 BoneTree에 추가
    BoneTree.Add(NewBoneNode);

    // 참조 스켈레톤 업데이트
    // ReferenceSkeleton.BoneInfo는 FBoneNode의 배열이므로, NewBoneNode의 복사본을 추가합니다.
    // 이 FBoneNode에는 로컬 BindTransform과 InverseGlobalBindTransform이 모두 포함됩니다.
    ReferenceSkeleton.BoneInfo.Add(NewBoneNode);

    ReferenceSkeleton.NameToIndexMap.Add(Name, ReferenceSkeleton.BoneInfo.Num() - 1);

    // RefBonePose에는 여전히 원본 글로벌 바인드 포즈를 저장 (GetGlobalBindTransform에서 직접 사용 위함)
    ReferenceSkeleton.RefBonePose.Add(InGlobalBindPose);

    // 현재 포즈 크기 조정
    CurrentPose.Resize(BoneTree.Num());
}
void USkeleton::AddBone(const FName Name, const FName ParentName, const FMatrix BindTransform, const FMatrix& InTransformMatrix)
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
    AddBone(Name, ParentIdx, BindTransform, InTransformMatrix);
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
        // 행 우선(vM): ChildGlobal = ChildLocal * ParentGlobal
        // 이 루프는 자식에서 부모로 올라가면서 로컬 변환들을 누적 곱함:
        // Global_new = Global_previously_calculated_for_child * Local_parent
        // 예: Bone2_Global = L2
        //      Bone1_Global_temp = L2 * L1  (Bone1이 Bone2의 부모)
        //      Bone0_Global_temp = (L2*L1) * L0 (Bone0이 Bone1의 부모)
        // 결과적으로 G = L_child * L_parent1 * L_parent2 * ... * L_root 형태가 됨.
        // 이는 올바른 글로벌 변환 계산 방식임 (루트부터 순차적으로 적용되는 것과 동일).
        Global = BoneTree[Parent].BindTransform * Global;
        Parent = BoneTree[Parent].ParentIndex;
    }
    return Global;
}

FMatrix USkeleton::GetInverseBindTransform(int32 BoneIdx) const
{
    if (BoneTree.IsValidIndex(BoneIdx))
    {
        return BoneTree[BoneIdx].InverseBindTransform; // 저장된 값 사용
    }
    return FMatrix::Identity;
}

FMatrix USkeleton::GetGeometryOffsetTransform(int32 BoneIdx) const
{
    if (BoneTree.IsValidIndex(BoneIdx))
    {
        return BoneTree[BoneIdx].GeometryOffsetMatrix; // 저장된 값 사용
    }
    return FMatrix::Identity;
}

FMatrix USkeleton::CalculateSkinningMatrix(int32 BoneIdx, const FMatrix& AnimationMatrix) const
{
    return  GetGeometryOffsetTransform(BoneIdx) * GetInverseBindTransform(BoneIdx) * AnimationMatrix;
}

void USkeleton::UpdateCurrentPose(const TArray<FMatrix>& LocalAnimationTransforms)
{
    // 현재 사용안함
    //// 로컬 변환 저장 
    //for (int32 i = 0; i < FMath::Min(LocalAnimationTransforms.Num(), CurrentPose.LocalTransforms.Num()); ++i)
    //{
    //    CurrentPose.LocalTransforms[i] = LocalAnimationTransforms[i];
    //}

    //// 글로벌 변환 계산
    //for (int32 i = 0; i < CurrentPose.LocalTransforms.Num(); ++i)
    //{
    //    if (BoneTree[i].ParentIndex == INDEX_NONE)
    //    {
    //        CurrentPose.GlobalTransforms[i] = CurrentPose.LocalTransforms[i];
    //    }
    //    else
    //    {
    //        CurrentPose.GlobalTransforms[i] = CurrentPose.GlobalTransforms[BoneTree[i].ParentIndex] * CurrentPose.LocalTransforms[i];
    //    }

    //    // 스키닝 행렬 계산
    //    CurrentPose.SkinningMatrices[i] = CalculateSkinningMatrix(i, CurrentPose.GlobalTransforms[i]);
    //}
}
