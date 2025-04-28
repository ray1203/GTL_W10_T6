#include "Wall.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Components/LuaScriptComponent.h"
#include "Components/TextComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

AWall::AWall()
{
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    TextComponent = AddComponent<UTextComponent>("TEXTCOMPONENT_0");
    TextComponent->SetupAttachment(RootComponent);
    TextComponent->SetTexture(L"Assets/Texture/font.png");
    TextComponent->SetRowColumnCount(106, 106);
    TextComponent->SetText(L"");

    // Initialize properties or components here
    SetActorLocation(FVector(20.0f, 0.0f, 0.0f));
}

UObject* AWall::Duplicate(UObject* InOuter)
{
    UObject* NewActor = Super::Duplicate(InOuter);
    AWall* PlayerCharacter = Cast<AWall>(NewActor);
    PlayerCharacter->TextComponent = GetComponentByFName<UTextComponent>("TEXTCOMPONENT_0");

    return NewActor;
}

void AWall::BeginPlay()
{
    Super::BeginPlay();
}

void AWall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWall::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AWall, ACharacter,
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation),
        "VarientValue", sol::property(&ThisClass::GetVarientValue, &ThisClass::SetVarientValue)
    )
}

bool AWall::BindSelfLuaProperties()
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

