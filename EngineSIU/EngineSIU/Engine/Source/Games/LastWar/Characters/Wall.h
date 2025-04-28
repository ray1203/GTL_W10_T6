#pragma once
#include "Components/TextComponent.h"
#include "GameFramework/Character.h"

class UTextComponent;

class AWall : public ACharacter
{
	DECLARE_CLASS(AWall, ACharacter)
public:
    AWall();
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    UObject* Duplicate(UObject* InOuter) override;

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    // === Getter ===
    float GetVarientValue() const
    {
        return VarientValue;
    }

    // === Setter ===
    void SetVarientValue(float InVarientValue)
    {
        VarientValue = InVarientValue;
        if (InVarientValue >= 0.f)
        {
            TextComponent->SetText(FString("더하기 " + std::to_string(InVarientValue)).ToWideString());
        }
        else
        {
            TextComponent->SetText(FString("빼기 " + std::to_string(InVarientValue)).ToWideString());
        }
    }

private:
    UPROPERTY
    (int32, VarientValue, = 0)

    UPROPERTY
    (UTextComponent*, TextComponent, = nullptr)
};
