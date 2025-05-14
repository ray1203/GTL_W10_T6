#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Delegates/DelegateCombination.h"
#include "EnemyCharacter.h"
#include "Wall.h"
#include "World/World.h"
#include "Components/LuaScriptComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/BoxComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "GameFramework/PlayerController.h"
#include "Games/LastWar/UI/LastWarUI.h"
#include "Audio/AudioManager.h"
#include "Camera/CameraModifier/CameraShakeModifier.h"
#include "Camera/PlayerCameraManager.h"
#include "Actors/CameraActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "AssetImporter/FBX/FLoaderFBX.h"
#include "Animation/AnimInstances/MyAnimInstance.h"
#include "AssetImporter/FBX/FBXManager.h"
#include "Animation/AnimSequence.h"

APlayerCharacter::APlayerCharacter()
{
    //BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Cube/cube-tex.obj"));
    Cast<USkeletalMeshComponent>(BodyMesh)->SetSkeletalMesh(FManagerFBX::GetSkeletalMesh(L"Contents/Mutant.fbx"));
    FManagerFBX::CreateAnimationAsset(L"Contents/Idle.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Walking.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Running.fbx");
    FManagerFBX::CreateAnimationAsset(L"Contents/Jumping.fbx");
    BodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    FollowCamera = AddComponent<UCameraComponent>("PlayerCamera");
    FollowCamera->SetupAttachment(RootComponent);
    FollowCamera->SetRelativeLocation(FVector(-3.0f, 0.0f, 1.5f));
}

UObject* APlayerCharacter::Duplicate(UObject* InOuter)
{
    UObject* NewActor = Super::Duplicate(InOuter);
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(NewActor);
    PlayerCharacter->FollowCamera = GetComponentByFName<UCameraComponent>("PlayerCamera");

    return NewActor;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    OnActorBeginOverlapHandle = OnActorBeginOverlap.AddDynamic(this, &APlayerCharacter::HandleOverlap);
    //SetCharacterMeshCount(1);

    UMyAnimInstance* AnimInstance = Cast<UMyAnimInstance>(Cast<USkeletalMeshComponent>(BodyMesh)->GetAnimInstance());
    if (AnimInstance == nullptr)
    {

        AnimInstance = FObjectFactory::ConstructObject<UMyAnimInstance>(nullptr);
        Cast<USkeletalMeshComponent>(BodyMesh)->SetAnimInstance(AnimInstance);
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
  
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateVerticalMovement(DeltaTime);

    if (!LastWarUI::bShowGameOver && Health <= 0)
    {
        EnableInput(Cast<APlayerController>(Controller));
        Health = 100.0f;
        SetCharacterMeshCount(1);
        SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
    }

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Bind input actions and axes here  
    PlayerInputComponent->BindAxis("MoveForward", [this](float Value) { MoveForward(Value); });
    PlayerInputComponent->BindAxis("MoveRight", [this](float Value) { MoveRight(Value); });
    PlayerInputComponent->BindAction("MoveForwardRelease", [this]() { MoveForwardRelease(); });
    PlayerInputComponent->BindAction("MoveRightRelease", [this]() { MoveRightRelease(); });
    PlayerInputComponent->BindAction("Jump", [this]() { Jump(); });
}

void APlayerCharacter::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Health"), std::to_string(Health));
    OutProperties.Add(TEXT("Speed"), std::to_string(Speed));
    OutProperties.Add(TEXT("AttackDamage"), std::to_string(AttackDamage));
}

void APlayerCharacter::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Health"));
    if (TempStr)
    {
        Health = std::stof(GetData(*TempStr));
    }
    TempStr = InProperties.Find(TEXT("Speed"));
    if (TempStr)
    {
        Speed = std::stof(GetData(*TempStr));
    }
    TempStr = InProperties.Find(TEXT("AttackDamage"));
    if (TempStr)
    {
        AttackDamage = std::stof(GetData(*TempStr));
    }
}

void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        if (Velocity <= MaxSpeed)
        {
            Velocity += 0.1f;
        }

        AddMovementInput(GetActorForwardVector(), Value);

        FVector Location = GetActorLocation();
        SetActorLocation(Location);

        if (Value >= 0.0f)
        {
            BodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        }
        else
        {
            BodyMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        }
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        if (Velocity <= MaxSpeed)
        {
            Velocity += 0.1f;
        }

        AddMovementInput(FVector::RightVector, Value);

        FVector Location = GetActorLocation();
        SetActorLocation(Location);

        if (Value >= 0.0f)
        {
            BodyMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
        }
        else
        {
            BodyMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
        }
    }
}

void APlayerCharacter::Jump()
{
    if (Controller && !bIsJumping)
    {
        bIsJumping = true;
        VerticalVelocity = 6.0f;
    }
}

void APlayerCharacter::UpdateVerticalMovement(float DeltaTime)
{
    if (!bCharacterVerticalPositionChange)
        return;

    // 중력 적용
    VerticalVelocity += GravityZ * DeltaTime;

    // 위치 계산
    FVector Loc = GetActorLocation();
    Loc.Z += VerticalVelocity * DeltaTime;

    // 바닥 체크
    if (Loc.Z <= 0.0f)
    {
        Loc.Z = 0.0f;
        VerticalVelocity = 0.0f;
        bIsJumping = false;
        bCharacterVerticalPositionChange = false;
    }
    SetActorLocation(Loc);
}

void APlayerCharacter::MoveForwardPress()
{
}

void APlayerCharacter::MoveForwardRelease()
{
    Velocity = InitVelocity;
}

void APlayerCharacter::MoveRightPress()
{
}

void APlayerCharacter::MoveRightRelease()
{
    Velocity = InitVelocity;
}

void APlayerCharacter::HandleOverlap(AActor* OtherActor)
{
    if (IsActorBeingDestroyed())  
    {  
        return;  
    }

    UE_LOG(LogLevel::Display, "Handle Overlap %s,  %s", GetData(OtherActor->GetName()), GetData(GetName()));
    
    if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor))  
    {  
        if (LuaScriptComponent)
        {
            LuaScriptComponent->ActivateFunction("OnOverlapEnemy", Enemy);
        }

        AddShakeModifier(0.5f, 0.1f, 0.1f, 1.0f);
 
        AudioManager::Get().PlayOneShot(EAudioType::Shot);
    }
    else if (AWall* Wall = Cast<AWall>(OtherActor))
    {
        if (LuaScriptComponent)
        {
            if(Wall->BoxComponent)
            {
                if (!Wall->BoxComponent->GetOverlapCheck())
                {
                    return;
                }
                Wall->BoxComponent->SetOverlapCheck(false);
            }
            LuaScriptComponent->ActivateFunction("OnOverlapWall", OtherActor, Wall->GetVarientValue());
        }

        AudioManager::Get().PlayOneShot(EAudioType::Shot);
    }

    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
        DisableInput(Cast<APlayerController>(Controller));

        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            if (PC->PlayerCameraManager)
            {
                FVector SpawnLoc = GetActorLocation();
                FRotator SpawnRot = FRotator(-89.0f, 0.0f, 0.0f);
                ACameraActor* CameraActor = PC->GetWorld()->SpawnActor<ACameraActor>();
                CameraActor->SetActorLocation(SpawnLoc);
                CameraActor->SetActorRotation(SpawnRot);

                FViewTargetTransitionParams BlendParams;
                BlendParams.BlendTime = 10.0f;
                BlendParams.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
                BlendParams.BlendExp = 2.0f;
                BlendParams.bLockOutgoing = false;

                //PC->UnPossess();
                PC->PlayerCameraManager->SetViewTarget(CameraActor, BlendParams);
                
            }
        }
    }
}

// PlayerCharacter.cpp
AActor* APlayerCharacter::SpawnActorLua(const std::string& ClassName, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    UClass* SpawnClass = UClass::FindClass(ClassName.c_str());
    if (!SpawnClass)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: Cannot find class '%s'"), ClassName);
        return nullptr;
    }

    AActor* NewActor = World->SpawnActor(SpawnClass);
    if (!NewActor)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActorLua: SpawnActor returned null for '%s'"), ClassName);
        return nullptr;
    }

    NewActor->SetActorLocation(Location);

    return NewActor;
}

void APlayerCharacter::AddShakeModifier(float Duration, float AlphaInTime, float AlphaOutTime, float Scale)
{
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        UCameraShakeModifier* ShakeModifier = FObjectFactory::ConstructObject<UCameraShakeModifier>(this);
        ShakeModifier->Duration = 0.5f;
        ShakeModifier->AlphaInTime = 0.1f;
        ShakeModifier->AlphaOutTime = 0.2f;
        ShakeModifier->Scale = 1.0f;

        PC->PlayerCameraManager->AddModifier(ShakeModifier);
    }
}


void APlayerCharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayerCharacter, sol::bases<AActor, APawn, ACharacter>(),
        "SpawnActorLua", &ThisClass::SpawnActorLua,
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation),
        "Health", sol::property(&ThisClass::GetHealth, &ThisClass::SetHealth),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "AttackDamage", sol::property(&ThisClass::GetAttackDamage, &ThisClass::SetAttackDamage),
        "AddCharacterMeshCount", &ThisClass::AddCharacterMeshCount,
        "CharacterMeshCount", sol::property(&ThisClass::GetCharacterMeshCount, &ThisClass::SetCharacterMeshCount)
    )
}

bool APlayerCharacter::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();
    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    return true;
}

void APlayerCharacter::AddCharacterMeshCount(int32 InCount)
{
    CharacterMeshCount += InCount;
    CharacterMeshCount = FMath::Max(1, CharacterMeshCount);
    SetCharacterMeshCount(CharacterMeshCount);
}

void APlayerCharacter::SetCharacterMeshCount(int32 InCount)
{
    CharacterMeshCount = InCount;
    CharacterMeshCount = FMath::Max(1, CharacterMeshCount);
    CharacterMeshCount = FMath::Min(CharacterMeshCount, 15);
    
    while (StaticMeshComponents.Num() < CharacterMeshCount)
    {
        UStaticMeshComponent* StaticMeshComponent = AddComponent<UStaticMeshComponent>();
        StaticMeshComponents.Add(StaticMeshComponent);
        StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Cube/cube-tex.obj"));
        StaticMeshComponent->SetupAttachment(RootComponent);
    }

    while (StaticMeshComponents.Num() > CharacterMeshCount)
    {
        UStaticMeshComponent* StaticMeshComponent = StaticMeshComponents[StaticMeshComponents.Num() - 1];

        StaticMeshComponents.RemoveAt(StaticMeshComponents.Num() - 1);
        

        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            if (EditorEngine->GetSelectedComponent() == StaticMeshComponent)
            {
                EditorEngine->DeselectComponent(StaticMeshComponent);
            }
        }
        
        // 순서 조심
        StaticMeshComponent->DestroyComponent();
    }


    constexpr float weight = .4f;
    for (int i = 0; i < StaticMeshComponents.Num(); i++)
    {
        float distance = FMath::Sqrt(static_cast<float>(i)) * weight;
        float cos = FMath::Cos(i * 100.f) * distance;
        float sin = FMath::Sin(i * 100.f) * distance;
        StaticMeshComponents[i]->SetRelativeLocation(FVector(cos, sin, StaticMeshComponents[i]->GetRelativeLocation().Z));
    }

    float Distance = 1 + FMath::Sqrt(static_cast<float>(StaticMeshComponents.Num())) * weight;
    CollisionCapsule->SetHalfHeight(Distance);
    CollisionCapsule->SetRadius(Distance);
}
 
