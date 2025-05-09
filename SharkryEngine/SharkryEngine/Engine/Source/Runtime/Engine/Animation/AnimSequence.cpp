#include "AnimSequence.h"
#include "Engine/Animation/AnimData/AnimDataModel.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

UAnimSequence::UAnimSequence()
{
}

void UAnimSequence::GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext)
{
    if (!AnimDataModel)
    {
        //  TODO 모델이 없으면 참조 포즈로 초기화
        /*OutAnimationPoseData.ResetToRefPose(
            OutAnimationPoseData.AnimInstanceProxy->GetRequiredBones()
        );*/
        return;
    }

    // 2) 로컬 포즈를 참조 포즈로 초기화
    /*OutAnimationPoseData.ResetToRefPose(
        OutAnimationPoseData.AnimInstanceProxy->GetRequiredBones()
    );*/

    // 3) 각 뼈 애니메이션 트랙을 샘플링하여 포즈에 덮어쓰기
    const TArray<FBoneAnimationTrack>& Tracks = AnimDataModel->GetBoneAnimationTracks();
    const TMap<FName, uint32>& BoneNameToIndex = OutAnimationPoseData.AnimInstance->GetRequiredBones();

    // 3) 각 트랙을 샘플링해서 Pose 배열에 덮어쓰기
    for (const FBoneAnimationTrack& Track : Tracks)
    {
        // 트랙의 Bone 이름으로 Pose 인덱스를 찾아본다
        const uint32* FoundIndex = BoneNameToIndex.Find(Track.Name);
        if (!FoundIndex)
        {
            continue;
        }
        uint32 PoseIndex = *FoundIndex;

        // 시간에 따른 트랜스폼 샘플링 (선형 보간)
        const FRawAnimSequenceTrack& Raw = Track.InternalTrack;
        const double LocalTime = ExtractionContext.CurrentTime;
        const double FrameRateD = AnimDataModel->FrameRate.AsDecimal();
        const double SamplePos = LocalTime * FrameRateD;

        const int32 Index0 = FMath::FloorToInt32(SamplePos);
        const int32 Index1 = FMath::Min(Index0 + 1, Raw.PosKeys.Num() - 1);
        const float Alpha = float(SamplePos - Index0);

        // 위치 보간
        const FVector P0 = Raw.PosKeys.IsValidIndex(Index0) ? Raw.PosKeys[Index0] : FVector::ZeroVector;
        const FVector P1 = Raw.PosKeys.IsValidIndex(Index1) ? Raw.PosKeys[Index1] : P0;
        FVector Translation = FMath::Lerp(P0, P1, Alpha);

        // 회전 SLERP
        const FQuat R0 = Raw.RotKeys.IsValidIndex(Index0) ? Raw.RotKeys[Index0] : FQuat();  // 기본생성자가 회전 없는 거라서
        const FQuat R1 = Raw.RotKeys.IsValidIndex(Index1) ? Raw.RotKeys[Index1] : R0;
        FQuat Rotation = FQuat::Slerp(R0, R1, Alpha);
        Rotation.Normalize();

        // 스케일 보간
        const FVector S0 = Raw.ScaleKeys.IsValidIndex(Index0) ? Raw.ScaleKeys[Index0] : FVector(1.f);
        const FVector S1 = Raw.ScaleKeys.IsValidIndex(Index1) ? Raw.ScaleKeys[Index1] : S0;
        FVector Scale = FMath::Lerp(S0, S1, Alpha);

        // 최종 트랜스폼 작성
        FMatrix BoneTransform = JungleMath::CreateModelMatrix(Translation, Rotation, Scale);

        // Pose에 적용
        OutAnimationPoseData.Pose.BoneTransforms[PoseIndex] = BoneTransform;
    }

    // TODO 4) 커브 데이터 평가
    //AnimDataModel->CurveData.Evaluate(ExtractionContext.CurrentTime, OutAnimationPoseData.Curve);
}
