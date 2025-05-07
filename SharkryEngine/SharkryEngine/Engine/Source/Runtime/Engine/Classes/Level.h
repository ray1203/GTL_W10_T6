#pragma once
#include <filesystem>

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;
class UWorld;


class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)

public:
    ULevel() = default;

    void InitLevel(UWorld* InOwningWorld);
    void Release();

    virtual UObject* Duplicate(UObject* InOuter) override;

    TArray<AActor*> Actors;
    UWorld* OwningWorld;

    FString GetLevelName() const { return LevelName; }
    FString GetLevelPath() const { return LevelPath; }
    void SetLevelPath(const std::filesystem::path& InLevelPath)
    {
        LevelName = FString(InLevelPath.stem());
        LevelPath = FString(InLevelPath);
    }

private:
    // Default Path는 엔진 시작시 LoadLevel에서 넣어짐.
    FString LevelName = "DefaultLevel";
    FString LevelPath = "Saved/DefaultLevel.scene";
};
