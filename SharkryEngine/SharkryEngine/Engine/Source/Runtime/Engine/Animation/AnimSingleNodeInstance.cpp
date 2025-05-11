#include "AnimSingleNodeInstance.h"
#include "AnimationAsset.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{
}

void UAnimSingleNodeInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (!AnimAsset)
    {
        UE_LOG(LogLevel::Warning, TEXT("UAnimSingleNodeInstance::InitializeAnimation: AnimAsset is null."));
        return;
    }

    CurrentPosition = 0.f;

    NativeUpdateAnimation(0.f);
}

void UAnimSingleNodeInstance::SetAnimationAsset(UAnimationAsset* NewAsset, bool bIsLooping, float InPlayRate)
{
    if (NewAsset == nullptr || NewAsset == AnimAsset)
    {
        return;
    }

    AnimAsset = NewAsset;
    bLooping = bIsLooping;
    PlayRate = InPlayRate;

    NativeInitializeAnimation();
}

void UAnimSingleNodeInstance::SetPlaying(bool bInPlaying)
{
    bPlaying = bInPlaying;
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!AnimAsset || !bPlaying)
    {
        return;
    }

    // 재생 위치 갱신
    CurrentPosition += DeltaSeconds * PlayRate;

    // 길이 초과 시 처리
    const float Length = AnimAsset->GetPlayLength();
    if (CurrentPosition >= Length)
    {
        if (bLooping)
        {
            CurrentPosition = FMath::Fmod(CurrentPosition, Length);
        }
        else
        {
            bPlaying = false;
            return;
        }
    }

    // 포즈 추출
    FAnimExtractContext ExtractContext(CurrentPosition, false);
    FPoseContext Pose(this);
    AnimAsset->GetAnimationPose(Pose, ExtractContext);

    // 결과 출력
    Output.Pose = Pose.Pose;
    Output.Curve = Pose.Curve;
}
