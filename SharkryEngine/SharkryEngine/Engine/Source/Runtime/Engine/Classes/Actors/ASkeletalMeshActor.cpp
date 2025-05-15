#include "ASkeletalMeshActor.h"

#include "AssetImporter/FBX/FBXManager.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstances/MyAnimInstance.h"
#include "Animation/AnimSequence.h"

ASkeletalMeshActor::ASkeletalMeshActor()
{
    SkeletalMeshComponent = Cast<USkeletalMeshComponent>(BodyMesh);
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
    FManagerFBX::CreateAnimationAsset(L"Contents/Sharkry_Idle.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Sharkry_Walking.fbx");
        FManagerFBX::CreateAnimationAsset(L"Contents/Sharkry_Dancing.fbx");
    SetActorTickInEditor(true);
    RootComponent = SkeletalMeshComponent;

    SetAnimationAsset();
    SkeletalMeshComponent->InitAnimation();
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
    UMyAnimInstance* AnimInstance = Cast<UMyAnimInstance>(SkeletalMeshComponent->GetAnimInstance());
    if (AnimInstance == nullptr)
    {
        AnimInstance = FObjectFactory::ConstructObject<UMyAnimInstance>(nullptr);
        SkeletalMeshComponent->SetAnimInstance(AnimInstance);
    }

    TMap<FString, UAnimationAsset*> AnimationAssets = FManagerFBX::GetAnimationAssets();
    for (auto& Anim : AnimationAssets)
    {
        if (Anim.Key == "Idle_mixamo.com")
        {
            AnimInstance->SetIdleAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Walking_mixamo.com")
        {
            AnimInstance->SetWalkAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Running_mixamo.com")
        {
            AnimInstance->SetRunAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
        else if (Anim.Key == "Jumping_mixamo.com")
        {
            AnimInstance->SetJumpAnimSequence(Cast<UAnimSequence>(Anim.Value));
        }
    }
    AnimInstance->NativeInitializeAnimation();
}
