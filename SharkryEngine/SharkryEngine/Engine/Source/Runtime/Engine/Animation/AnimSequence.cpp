#include "Math/JungleMath.h"
#include "AnimSequence.h"
#include "AnimData/AnimDataModel.h"
#include "AnimNotify.h"
#include "AnimInstance.h"

UAnimSequence::UAnimSequence()
{
    GenerateTestCode();
}

float UAnimSequence::GetPlayLength()
{
    if (AnimDataModel == nullptr) {
        return 0.0f;
    }

    return AnimDataModel->PlayLength;
}

void UAnimSequence::GenerateTestCode()
{
    FName TestAName = "TestA";
    FAnimNotifyEvent TestA = FAnimNotifyEvent(0.5f, 0.0f, TestAName);
    Notifies.Add(TestA);

    FName TestBName = "TestB";
    FAnimNotifyEvent TestB = FAnimNotifyEvent(0.7f, 1.0f, TestBName, ENotifyMode::State, ENotifyState::None);
    Notifies.Add(TestB);
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
    OutAnimationPoseData.ResetToRefPose(
        OutAnimationPoseData.AnimInstance->GetRequiredBoneLocalTransforms()
    );

    // 3) 각 뼈 애니메이션 트랙을 샘플링하여 포즈에 덮어쓰기
    const TArray<FBoneAnimationTrack>& Tracks = AnimDataModel->GetBoneAnimationTracks();
    const TMap<FName, uint32>& BoneNameToIndex = OutAnimationPoseData.AnimInstance->GetRequiredBoneNames();

    // 3) 각 트랙을 샘플링해서 Pose 배열에 덮어쓰기
    for (const FBoneAnimationTrack& Track : Tracks)
    {
        const uint32* FoundIndex = BoneNameToIndex.Find(Track.Name);
        if (!FoundIndex)
        {
            continue;
        }
        uint32 PoseIndex = *FoundIndex;

        // Default 값, 해당 프레임 데이터가 없으면 사용할 것
        FMatrix& DefaultBoneTransform = OutAnimationPoseData.Pose.BoneTransforms[PoseIndex];
        FVector DefaultPos = JungleMath::DecomposeTranslation(DefaultBoneTransform);
        FQuat DefulatRot = JungleMath::DecomposeRotation(DefaultBoneTransform);
        FVector DefaultScale = JungleMath::DecomposeScale(DefaultBoneTransform);


        // 시간에 따른 트랜스폼 샘플링 (선형 보간)
        const FRawAnimSequenceTrack& Raw = Track.InternalTrack;
        const double LocalTime = ExtractionContext.CurrentTime;
        const double FrameRateD = AnimDataModel->FrameRate.AsDecimal();
        const double SamplePos = LocalTime * FrameRateD;

        const int32 Index0 = FMath::FloorToInt32(SamplePos);
        const int32 Index1 = FMath::Min(Index0 + 1, Raw.PosKeys.Num() - 1);
        const float Alpha = float(SamplePos - Index0);

        // 위치 보간
        const FVector P0 = Raw.PosKeys.IsValidIndex(Index0) ? Raw.PosKeys[Index0] : DefaultPos;
        const FVector P1 = Raw.PosKeys.IsValidIndex(Index1) ? Raw.PosKeys[Index1] : P0;
        FVector Translation = FMath::Lerp(P0, P1, Alpha);

        // 회전 SLERP
        const FQuat R0 = Raw.RotKeys.IsValidIndex(Index0) ? Raw.RotKeys[Index0] : DefulatRot;  // 기본생성자가 회전 없는 거라서
        const FQuat R1 = Raw.RotKeys.IsValidIndex(Index1) ? Raw.RotKeys[Index1] : R0;
        FQuat Rotation = FQuat::Slerp(R0, R1, Alpha);
        // Slerp에 Normalize가 있음

        // 스케일 보간
        const FVector S0 = Raw.ScaleKeys.IsValidIndex(Index0) ? Raw.ScaleKeys[Index0] : DefaultScale;
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

void UAnimSequence::GetAnimNotifies(const float& StartTime, const float& DeltaTime, const bool bAllowLooping, TArray<FAnimNotifyEvent*>& OutNotifies)
{
    if ((Notifies.Num() == 0) || (DeltaTime == 0.f))
    {
        return;
    }

    const bool bPlayingBackward = (DeltaTime < 0.f);

    float CurrentTime = StartTime + DeltaTime;

    for (FAnimNotifyEvent& Notify : Notifies)
    {
        // 단일 Notify 인 경우
        if (Notify.NotifyMode == ENotifyMode::Single)
        {
            if (StartTime < Notify.TriggerTime && Notify.TriggerTime <= CurrentTime)
            {
                OutNotifies.Add(&Notify);
            }
        }
        else // State Notify 인 경우
        {
            // 겹치기만 해도 Add 해버리기
            if (!((Notify.TriggerTime + Notify.Duration) < StartTime || (CurrentTime < Notify.TriggerTime)))
            {
                OutNotifies.Add(&Notify);
            }
        }
    }

    float EndTime = AnimDataModel->PlayLength;

    // 만약 Animation이 끝났는데 Looping이 걸려 있다면
    if (CurrentTime > EndTime && bAllowLooping)
    {
        CurrentTime = CurrentTime - EndTime;

        for (FAnimNotifyEvent& Notify : Notifies)
        {
            // 단일 Notify 인 경우
            if (Notify.NotifyMode == ENotifyMode::Single)
            {
                if (0.0f <= Notify.TriggerTime && Notify.TriggerTime <= CurrentTime)
                {
                    OutNotifies.Add(&Notify);
                }
            }
            else // State Notify 인 경우
            {
                // UX 상 애니메이션이 끝나는데 End가 불릴 것을 기대할 것으로 보이므로
                // 추가하지 않아서 End가 불리도록 유도
            }
        }
    }
}
