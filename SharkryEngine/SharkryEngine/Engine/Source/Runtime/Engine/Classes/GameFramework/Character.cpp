#include "Character.h"  
#include "Components/InputComponent.h"  
#include "Controller.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Animation/AnimNotify.h"

ACharacter::ACharacter()  
{  
    CollisionCapsule = AddComponent<UCapsuleComponent>("CollisionCapsule");
    RootComponent = CollisionCapsule;
    BodyMesh = AddComponent<USkeletalMeshComponent>("SkeletalBodyMesh");
    BodyMesh->SetupAttachment(RootComponent);
}

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    UObject* NewActor = Super::Duplicate(InOuter);
    ACharacter* PlayerCharacter = Cast<ACharacter>(NewActor);
    PlayerCharacter->BodyMesh = GetComponentByFName<USkeletalMeshComponent>("SkeletalBodyMesh");

    return NewActor;
}

void ACharacter::BeginPlay()  
{  
    Super::BeginPlay();  
}  

void ACharacter::Tick(float DeltaTime)  
{  
    Super::Tick(DeltaTime);  
}

void ACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)  
{  
    Super::SetupPlayerInputComponent(PlayerInputComponent);  
}  

void ACharacter::PossessedBy(AController* NewController)  
{  
    Super::PossessedBy(NewController);  
}  

void ACharacter::UnPossessed()  
{  
    Super::UnPossessed();  
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>());
}

bool ACharacter::BindSelfLuaProperties()
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

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& Notify)
{
    FString State = "n";

    switch (Notify.NotifyState) 
    {
    case ENotifyState::None:
        State = "None";
        break;
    case ENotifyState::Begin:
        State = "Begin";
        break;
    case ENotifyState::Tick:
        State = "Tick";
        break;
    case ENotifyState::End:
        State = "End";
        break;
    }

    /*UE_LOG(LogLevel::Display,
        TEXT("FAnimNotifyEvent: Time=%.3f, Duration=%.3f, Name=%s, State=%s"),
        Notify.TriggerTime,
        Notify.Duration,
        *Notify.NotifyName.ToString(),
        *State
    );*/
}
