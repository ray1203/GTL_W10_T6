#include "ASkeletalMeshActor.h"
#include "FLoaderFBX.h"
#include "Components/SkeletalMeshComponent.h"

ASkeletalMeshActor::ASkeletalMeshActor()
{
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent_0");
    SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Mutant.fbx"));
    //SkeletalMeshComponent->PlayAnimation(FManagerFBX::GetAnimSequence(L"Contents/Mutant.fbx"), true);
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/RidenEi-V1.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/SharkryPVM_NoFixed.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Sharkry_Unreal.fbx"));
    //SkeletalMeshComponent->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Sharkry_NoTwist.fbx"));
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
