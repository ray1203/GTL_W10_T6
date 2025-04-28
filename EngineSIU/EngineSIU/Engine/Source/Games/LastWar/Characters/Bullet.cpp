#include "Bullet.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/LuaScriptComponent.h"

UObject* ABullet::Duplicate(UObject* InOuter)
{
    ABullet* NewBullet = Cast<ABullet>(Super::Duplicate(InOuter));
    
    return NewBullet;
}

void ABullet::BeginPlay()
{
    Super::BeginPlay();

    OnActorBeginOverlapHandle = OnActorBeginOverlap.AddDynamic(this, &ABullet::OnBeginOverlap);
}

void ABullet::InitBullet(float InBulletSpeed, float InBulletDamage)
{
    BulletSpeed = InBulletSpeed;
    BulletDamage = InBulletDamage;
}

void ABullet::OnBeginOverlap(AActor* OtherActor)
{
    if (OtherActor == this)
    {
        return;
    }
    if (LuaScriptComponent)
    {
        LuaScriptComponent->ActivateFunction("OnBeginOverlap", OtherActor);
    }
}

void ABullet::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ABullet, sol::bases<AActor>(),
        "InitBullet", &ABullet::InitBullet,
        "BulletSpeed", sol::property(&ABullet::GetBulletSpeed, &ABullet::SetBulletSpeed),
        "BulletDamage", sol::property(&ABullet::GetBulletDamage, &ABullet::SetBulletDamage)
        );
}

bool ABullet::BindSelfLuaProperties()
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
