#pragma once
#include "Components/SkinnedMeshComponent.h"
#include "Mesh/SkeletalMesh.h"

enum class EAnimationMode {
    AnimationBlueprint,
    AnimationSingleNode,
    AnimationCustomMode
};;

class UAnimSingleNodeInstance;
struct FAnimNotifyEvent;
class UAnimSequence;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

public:
    USkeletalMeshComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void TickComponent(float DeltaTime) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; };

    void SetAnimAsset(const FString& AnimName);

    void TickAnimation(float DeltaTime, bool bNeedsValidRootMotion);
    void RefreshBoneTransforms();
    void HandleAnimNotify(const FAnimNotifyEvent& Notify);

    void PlayAnimation(UAnimSequence* NewAnimToPlay, bool bLooping = false, float BlendDuration = 0.5f, float InPlayRate = 1.f);
    void SetAnimationMode(EAnimationMode NewMode);
    EAnimationMode GetAnimationMode() const;
    void SetAnimation(UAnimSequence* NewAnimToPlay, float BlendDuration = 0.5f, float InPlayRate = 1.f);
    void Play(bool bLooping = false);
    void Stop();
    UAnimSingleNodeInstance* GetSingleNodeInstance();
private:
    EAnimationMode AnimationMode = EAnimationMode::AnimationSingleNode;
    UAnimSingleNodeInstance* AnimInstance = nullptr;
};
