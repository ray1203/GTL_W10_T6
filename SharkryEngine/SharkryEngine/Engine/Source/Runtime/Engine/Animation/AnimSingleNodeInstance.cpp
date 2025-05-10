#include "AnimSingleNodeInstance.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{
}

void UAnimSingleNodeInstance::InitializeAnimation()
{
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

    InitializeAnimation();
}

void UAnimSingleNodeInstance::SetPlaying(bool bInPlaying)
{
    bPlaying = bInPlaying;
}
