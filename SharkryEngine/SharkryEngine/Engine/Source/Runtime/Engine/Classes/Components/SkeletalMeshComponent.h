#pragma once
#include "Components/SkinnedMeshComponent.h"
#include "Mesh/SkeletalMesh.h"
#include "Container/Array.h"

enum class EAnimationMode {
    AnimationBlueprint,
    AnimationSingleNode,
    AnimationCustomMode
};;

class UAnimInstance;
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

    void TickAnimation(float DeltaTime, bool bNeedsValidRootMotion);
    void RefreshBoneTransforms();
    void HandleAnimNotify(const FAnimNotifyEvent& Notify);

    void InitAnimation();
    void PlayAnimation(EAnimationMode NewAnimMode, UAnimSequence* NewAnimToPlay, bool bLooping = false);
    void SetAnimationMode(EAnimationMode NewMode);
    EAnimationMode GetAnimationMode() const;
    void SetAnimation(UAnimSequence* NewAnimToPlay, float BlendDuration = 0.7f, float InPlayRate = 1.f);
    void Play(bool bLooping = false);
    void Stop();
    UAnimSingleNodeInstance* GetSingleNodeInstance();

    void SetAnimInstance(UAnimInstance* InAnimInstance);
    UAnimInstance* GetAnimInstance();

    void AddAnimAssetName(FString AnimAssetName);
    TArray<FString> GetAnimAssetNames() { return AnimAssetNames; }
private:
    EAnimationMode AnimationMode = EAnimationMode::AnimationSingleNode;
    UAnimInstance* AnimInstance = nullptr;
    UAnimSingleNodeInstance* SingleNodeInstance = nullptr;
    TArray<FString> AnimAssetNames;
};
