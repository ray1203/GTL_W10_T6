#pragma once

#include "ActorComponent.h"
#include "sol/sol.hpp"

#pragma comment( linker, "/entry:WinMainCRTStartup /subsystem:console" )


class AActor;

class ULuaScriptComponent : public UActorComponent
{
    DECLARE_CLASS(ULuaScriptComponent, UActorComponent)

public:
    ULuaScriptComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;

    // 기본 함수는 자체적으로 불러지도록 세팅.
    // 추후 특정 Actor에서 함수를 추가 실행할 때는 Component->ActivateFunction()으로 호출해주기.
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void DestroyComponent(bool bPromoteChildren = false) override;

public:
    FString GetScriptName() const { return ScriptName; }
    bool LoadScript();

    template<typename... Args>
    void ActivateFunction(const FString& FunctionName, Args&&... args);

    sol::table& GetLuaSelfTable() { return SelfTable; }

private:
    FString ScriptName;
    sol::table SelfTable;
};

template<typename ...Args>
inline void ULuaScriptComponent::ActivateFunction(const FString& FunctionName, Args && ...args)
{
    if (SelfTable.valid() && SelfTable[*FunctionName].valid())
    {
        auto Result = SelfTable[*FunctionName](SelfTable, std::forward<Args>(args)...);
        if (!Result.valid())
        {
            sol::error err = Result;
            UE_LOG(LogLevel::Error, TEXT("Lua Error: %s"), *FString(err.what()));
        }
    }
}
