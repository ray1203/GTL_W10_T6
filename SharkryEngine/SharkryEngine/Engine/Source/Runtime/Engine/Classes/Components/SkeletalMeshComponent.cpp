#include "Components/SkeletalMeshComponent.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Launch/EngineLoop.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstances/AnimSingleNodeInstance.h"
#include "Engine/Classes/GameFramework/Character.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimNotify.h"

UObject* USkeletalMeshComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    // NewComponent->SkeletalMesh = SkeletalMesh;
    NewComponent->selectedSubMeshIndex = selectedSubMeshIndex;
    NewComponent->SkeletalMesh = SkeletalMesh;
    return NewComponent;
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    TickAnimation(DeltaTime, false);
}

void USkeletalMeshComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    //StaticMesh 경로 저장
    USkeletalMesh* CurrentMesh = GetSkeletalMesh();
    if (CurrentMesh != nullptr) {

        // 1. std::wstring 경로 얻기
        std::wstring PathWString = CurrentMesh->GetObjectName();

        // 2. std::wstring을 FString으로 변환
        FString PathFString(PathWString.c_str()); // c_str()로 const wchar_t* 얻어서 FString 생성
        // PathFString = CurrentMesh->ConvertToRelativePathFromAssets(PathFString);

        FWString PathWString2 = PathFString.ToWideString();


        OutProperties.Add(TEXT("SkeletalMeshPath"), PathFString);
    }
    else
    {
        OutProperties.Add(TEXT("SkeletalMeshPath"), TEXT("None")); // 메시 없음 명시
    }
}

void USkeletalMeshComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;


    // --- StaticMesh 설정 ---
    TempStr = InProperties.Find(TEXT("SkeletalMeshPath"));
    if (TempStr) // 키가 존재하는지 확인
    {
        if (*TempStr != TEXT("None")) // 값이 "None"이 아닌지 확인
        {
            // 경로 문자열로 UStaticMesh 에셋 로드 시도

            if (USkeletalMesh* MeshToSet = FManagerFBX::CreateSkeletalMesh(*TempStr))
            {
                SetSkeletalMesh(MeshToSet); // 성공 시 메시 설정
                UE_LOG(LogLevel::Display, TEXT("Set StaticMesh '%s' for %s"), **TempStr, *GetName());
            }
            else
            {
                // 로드 실패 시 경고 로그
                UE_LOG(LogLevel::Warning, TEXT("Could not load StaticMesh '%s' for %s"), **TempStr, *GetName());
                SetSkeletalMesh(nullptr); // 안전하게 nullptr로 설정
            }
        }
        else // 값이 "None"이면
        {
            SetSkeletalMesh(nullptr); // 명시적으로 메시 없음 설정
            UE_LOG(LogLevel::Display, TEXT("Set StaticMesh to None for %s"), *GetName());
        }
    }
    else // 키 자체가 없으면
    {
        // 키가 없는 경우 어떻게 처리할지 결정 (기본값 유지? nullptr 설정?)
        // 여기서는 기본값을 유지하거나, 안전하게 nullptr로 설정할 수 있습니다.
        // SetStaticMesh(nullptr); // 또는 아무것도 안 함
        UE_LOG(LogLevel::Display, TEXT("StaticMeshPath key not found for %s, mesh unchanged."), *GetName());
    }
}
 
void USkeletalMeshComponent::SetAnimAsset(const FString& AnimName)
{
    if (AnimInstance == nullptr) 
    {
        // 이후 SingleNode만 사용하지 않는 경우 수정 필요
        AnimInstance = FObjectFactory::ConstructObject<UAnimSingleNodeInstance>(nullptr);
        AnimInstance->SetSkeletalMesh(SkeletalMesh);
        AnimInstance->SetSkeletalMeshComponent(this);
    }

    TArray<UAnimationAsset*> AnimationAsset = FManagerFBX::GetAnimationAssets(AnimName);
    for (auto& Anim : AnimationAsset)
    {
        if (Anim == nullptr) continue;
        
        if (Anim->GetAssetPath() == "Contents/Idle.fbx")
        {
            AnimInstance->SetIdleAnimSequence(Cast<UAnimSequence>(Anim));
        }
        else if (Anim->GetAssetPath() == "Contents/Walking.fbx")
        {
            AnimInstance->SetWalkAnimSequence(Cast<UAnimSequence>(Anim));
        }
        else if (Anim->GetAssetPath() == "Contents/Running.fbx")
        {
            AnimInstance->SetRunAnimSequence(Cast<UAnimSequence>(Anim));
        }
        else if (Anim->GetAssetPath() == "Contents/Jumping.fbx")
        {
            AnimInstance->SetJumpAnimSequence(Cast<UAnimSequence>(Anim));
        }
        else
        {
            AnimInstance->SetIdleAnimSequence(Cast<UAnimSequence>(Anim));
        }
    }

    AnimInstance->NativeInitializeAnimation();
}

void USkeletalMeshComponent::TickAnimation(float DeltaTime, bool bNeedsValidRootMotion)
{
    if (AnimInstance != nullptr) 
    {
        AnimInstance->UpdateAnimation(DeltaTime);
        RefreshBoneTransforms();
    }
}

void USkeletalMeshComponent::RefreshBoneTransforms()
{
    const FPoseContext& AnimPose = AnimInstance->GetOutput();
    
    for (int i = 0; i < AnimPose.Pose.BoneTransforms.Num(); i++) 
    {
        SkeletalMesh->SetBoneLocalMatrix(i, AnimPose.Pose.BoneTransforms[i]);
    }

    SkeletalMesh->UpdateWorldTransforms();
    UpdateAndApplySkinning();
}

void USkeletalMeshComponent::HandleAnimNotify(const FAnimNotifyEvent& Notify)
{
    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (CharacterOwner)
    {
        CharacterOwner->HandleAnimNotify(Notify);
    }
}

void USkeletalMeshComponent::PlayAnimation(UAnimSequence* NewAnimToPlay, bool bLooping)
{
    SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SetAnimation(NewAnimToPlay);
    Play(bLooping);
}

void USkeletalMeshComponent::SetAnimationMode(EAnimationMode NewMode)
{
    AnimationMode = NewMode;
}

EAnimationMode USkeletalMeshComponent::GetAnimationMode() const
{
    return AnimationMode;
}

void USkeletalMeshComponent::SetAnimation(UAnimSequence* NewAnimToPlay)
{
    UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance();
    if (SingleNodeInstance)
    {
        SingleNodeInstance->SetSkeletalMesh(SkeletalMesh);
        SingleNodeInstance->SetAnimationSequence(NewAnimToPlay, true);
        SingleNodeInstance->SetPlaying(false);
    }
}

void USkeletalMeshComponent::Play(bool bLooping)
{
    UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance();
    if (SingleNodeInstance)
    {
        SingleNodeInstance->SetPlaying(true);
    }
}

void USkeletalMeshComponent::Stop()
{
    UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance();
    if (SingleNodeInstance)
    {
        SingleNodeInstance->SetPlaying(false);
    }
}

UAnimSingleNodeInstance* USkeletalMeshComponent::GetSingleNodeInstance()
{
    return Cast<UAnimSingleNodeInstance>(AnimInstance);
}
