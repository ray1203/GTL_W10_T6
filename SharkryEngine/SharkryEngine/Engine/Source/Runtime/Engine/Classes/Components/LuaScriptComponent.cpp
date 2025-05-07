#include "LuaScriptComponent.h"

#include "Engine/Lua/LuaScriptManager.h"
#include "GameFramework/Actor.h"

#include "World/World.h"
#include "Level.h"


ULuaScriptComponent::ULuaScriptComponent()
{
}

UObject* ULuaScriptComponent::Duplicate(UObject* InOuter)
{
    ULuaScriptComponent* NewComponent = Cast<ULuaScriptComponent>(Super::Duplicate(InOuter));

    if (!NewComponent)
        return nullptr;

    NewComponent->ScriptName = ScriptName;
    NewComponent->SelfTable = SelfTable;
 
    return NewComponent;
}

void ULuaScriptComponent::InitializeComponent()
{
    if (HasBeenInitialized())
    {
        return;
    }
    Super::InitializeComponent();
    if (ScriptName.IsEmpty())
    {
        if (GetWorld() && GetWorld()->GetActiveLevel())
        {
            FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
            ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());
        }
    }

    FLuaScriptManager::Get().RegisterActiveLuaComponent(this);
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (ScriptName.IsEmpty())
    {
        FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
        ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());
    }

    if (SelfTable.valid() && SelfTable["BeginPlay"].valid())
    {
        ActivateFunction("BeginPlay", SelfTable);
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (SelfTable.valid() && SelfTable["Tick"].valid())
    {
        ActivateFunction("Tick", DeltaTime);
    }
}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SelfTable.valid() && SelfTable["EndPlay"].valid())
    {
        ActivateFunction("EndPlay", EndPlayReason);
    }
}

void ULuaScriptComponent::DestroyComponent(bool bPromoteChildren)
{
    FLuaScriptManager::Get().UnRigisterActiveLuaComponent(this);
    Super::DestroyComponent(bPromoteChildren);
}

bool ULuaScriptComponent::LoadScript()
{
    if (ScriptName.IsEmpty())
    {
        FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
        ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());
    }

    SelfTable = FLuaScriptManager::Get().CreateLuaTable(ScriptName);

    if (!SelfTable.valid())
    {
        return false;
    }

    return true;
}
