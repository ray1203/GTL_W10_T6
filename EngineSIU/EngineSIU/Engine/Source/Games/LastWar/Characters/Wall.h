#pragma once
#include "Components/TextComponent.h"
#include "Engine/StaticMeshActor.h"

class UBoxComponent;

class AWall : public AStaticMeshActor
{
	DECLARE_CLASS(AWall, AStaticMeshActor)
public:
    AWall();
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    UObject* Duplicate(UObject* InOuter) override;

    /**
        * 해당 액터의 직렬화 가능한 속성들을 문자열 맵으로 반환합니다.
        * 하위 클래스는 이 함수를 재정의하여 자신만의 속성을 추가해야 합니다.
        */
    void GetProperties(TMap<FString, FString>& OutProperties) const override;

    /** 저장된 Properties 맵에서 액터의 상태를 복원합니다. */
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    // === Getter ===
    int32 GetVarientValue() const { return VarientValue; }
    float GetSpeed() const { return Speed; }

    // === Setter ===
    void SetVarientValue(int32 InVarientValue)
    {
        VarientValue = InVarientValue;
        if (VarientValue >= 0)
        {
            TextComponent->SetText(FString("더하기 " + std::to_string(InVarientValue)).ToWideString());
        }
        else
        {
            TextComponent->SetText(FString("빼기 " + std::to_string(FMath::Abs(InVarientValue))).ToWideString());
        }
    }
    void SetSpeed(float NewSpeed) { Speed = NewSpeed; }

private:
    UPROPERTY
    (int32, VarientValue, = 0)

    UPROPERTY
    (float, Speed, = 5.0f)

    UPROPERTY
    (UTextComponent*, TextComponent, = nullptr)

    // 임시 Temp임
public:
    UPROPERTY
    (UBoxComponent*, BoxComponent, = nullptr)
};
