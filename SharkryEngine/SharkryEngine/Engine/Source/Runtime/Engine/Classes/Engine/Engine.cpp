#include "Engine.h"

#include "UnrealEd/SceneManager.h"
#include "UObject/Casts.h"
#include "World/World.h"

UEngine* GEngine = nullptr;

void UEngine::Init()
{
}

bool UEngine::TryQuit(bool& OutbIsSave)
{
    return true;
}

FWorldContext* UEngine::GetWorldContextFromWorld(const UWorld* InWorld)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->World() == InWorld)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
    FWorldContext* NewWorldContext = new FWorldContext();
    WorldList.Add(NewWorldContext);
    NewWorldContext->WorldType = InWorldType;
    NewWorldContext->ContextHandle = FName(*FString::Printf(TEXT("WorldContext_%d"), NextWorldContextHandle++));

    return *NewWorldContext;
}
