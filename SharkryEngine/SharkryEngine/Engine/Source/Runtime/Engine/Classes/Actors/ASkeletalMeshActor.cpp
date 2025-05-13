#include "ASkeletalMeshActor.h"

#include "AssetImporter/FBX/FBXManager.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstances/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"

ASkeletalMeshActor::ASkeletalMeshActor()
{
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent_0");
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Mutant.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Capoeira.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/RidenEi-V1.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/SharkryPVM_NoFixed.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Sharkry_Unreal.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Sharkry_NoTwist.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/rp_nathan_animated_003_walking.fbx"));
    SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Mutant.fbx"));
    FManagerFBX::CreateAnimationAsset(L"Contents/Idle.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Walking.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Running.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Jumping.fbx");
    SetActorTickInEditor(true);
    RootComponent = SkeletalMeshComponent;

    SetAnimationAsset();
}

UObject* ASkeletalMeshActor::Duplicate(UObject* InOuter)
{
    UObject* NewActor = AActor::Duplicate(InOuter);
    ASkeletalMeshActor* NewSkeletalMeshActor = Cast<ASkeletalMeshActor>(NewActor);
    NewSkeletalMeshActor->SkeletalMeshComponent = GetComponentByFName<USkeletalMeshComponent>("SkeletalMeshComponent_0");

    return NewActor;
}

void ASkeletalMeshActor::BeginPlay()
{
    Super::BeginPlay();
}

USkeletalMeshComponent* ASkeletalMeshActor::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}

void ASkeletalMeshActor::SetSkeletalMesh(const FWString& SkelName)
{
    SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(SkelName));
}

void ASkeletalMeshActor::SetAnimationAsset()
{
    UAnimSingleNodeInstance* AnimInstance = SkeletalMeshComponent->GetSingleNodeInstance();
    if (AnimInstance == nullptr)
    {
        // 이후 SingleNode만 사용하지 않는 경우 수정 필요
        AnimInstance = FObjectFactory::ConstructObject<UAnimSingleNodeInstance>(nullptr);
        SkeletalMeshComponent->SetAnimInstance(AnimInstance);
    }

    TMap<FString, UAnimationAsset*> AnimationAssets = FManagerFBX::GetAnimationAssets();
    for (auto& Anim : AnimationAssets)
    {
        if (Anim.Key == "Contents/Idle.fbx")
        {
            AnimInstance->SetIdleAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Contents/Walking.fbx")
        {
            AnimInstance->SetWalkAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Contents/Running.fbx")
        {
            AnimInstance->SetRunAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Contents/Jumping.fbx")
        {
            AnimInstance->SetJumpAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else
        {
            AnimInstance->SetIdleAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }

    }
    AnimInstance->NativeInitializeAnimation();
}
