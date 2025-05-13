#include "ASkeletalMeshActor.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Components/SkeletalMeshComponent.h"

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
    SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Capoeira.fbx"));
    SkeletalMeshComponent->SetAnimAsset("Contents/Capoeira.fbx");
    //FManagerFBX::CreateAnimationAsset(L"Contents/Idle.fbx", L"Contents/Mutant.fbx");
    //FManagerFBX::CreateAnimationAsset(L"Contents/Walking.fbx", L"Contents/Mutant.fbx");
    //FManagerFBX::CreateAnimationAsset(L"Contents/Running.fbx", L"Contents/Mutant.fbx");
    //FManagerFBX::CreateAnimationAsset(L"Contents/Jumping.fbx", L"Contents/Mutant.fbx");
    //SkeletalMeshComponent->SetAnimAsset("Contents/Mutant.fbx");
    SetActorTickInEditor(true);
    RootComponent = SkeletalMeshComponent;
}

UObject* ASkeletalMeshActor::Duplicate(UObject* InOuter)
{
    UObject* NewActor = AActor::Duplicate(InOuter);
    ASkeletalMeshActor* NewSkeletalMeshActor = Cast<ASkeletalMeshActor>(NewActor);
    NewSkeletalMeshActor->SkeletalMeshComponent = GetComponentByFName<USkeletalMeshComponent>("SkeletalMeshComponent_0");

    return NewActor;
}

USkeletalMeshComponent* ASkeletalMeshActor::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}

void ASkeletalMeshActor::SetSkeletalMesh(const FWString& SkelName)
{
    SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(SkelName));
}
