#pragma once
#include "Components/SkinnedMeshComponent.h"
#include "Mesh/SkeletalMesh.h"

enum class EAnimationMode {
    AnimationBlueprint,
    AnimationSingleNode,
    AnimationCustomMode
};;

class UAnimInstance;
class UAnimSingleNodeInstance;
class UAnimationAsset;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

public:
    USkeletalMeshComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void TickComponent(float DeltaTime) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; };

    void PlayAnimation(UAnimationAsset* NewAnimToPlay, bool bLooping = false);
    void SetAnimationMode(EAnimationMode NewMode);
    EAnimationMode GetAnimationMode() const;
    void SetAnimation(UAnimationAsset* NewAnimToPlay);
    void Play(bool bLooping = false);
    void Stop();

    void TickAnimation(float DeltaTime);

    UAnimSingleNodeInstance* GetSingleNodeInstance();
private:
    EAnimationMode AnimationMode = EAnimationMode::AnimationSingleNode;
    UAnimationAsset* CurrentAnim = nullptr;
    UAnimInstance* AnimScriptInstance = nullptr;

};
