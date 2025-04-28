#include "Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Delegates/DelegateCombination.h"
#include "World/World.h"
#include "PlayerController.h"
#include "Components/InputComponent.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaScriptManager.h"

#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

void AActor::PostSpawnInitialize()
{
    InitLuaScriptComponent();
}

UObject* AActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->Owner = Owner;
    NewActor->bTickInEditor = bTickInEditor;
    // 기본적으로 있던 컴포넌트 제거
    TSet CopiedComponents = NewActor->OwnedComponents;

    //임시용 디폴트 컴포넌트 이름 저장
    //TODO: 디퐅트 컴포넌트를 삭제하지 않고 그 컴포넌트에 프로퍼티를 복사하는 방법으로 변경 필요
    TArray<FName> DefaultCopiedComponentNames;
    for (UActorComponent* Component : CopiedComponents)
    {
        DefaultCopiedComponentNames.Add(Component->GetFName());
        Component->DestroyComponent();
    }
    NewActor->OwnedComponents.Empty();


    // 부모-자식 관계 저장용 맵
    TMap<USceneComponent*, USceneComponent*> ParentChildMap;

    // 컴포넌트 복제 및 부모-자식 관계 추적
    TMap<USceneComponent*, USceneComponent*> OriginalToDuplicateMap; // 원본 -> 복제된 컴포넌트 매핑

    for (UActorComponent* Component : OwnedComponents)
    {
        UActorComponent* NewComponent = Cast<UActorComponent>(Component->Duplicate(NewActor));
        NewComponent->OwnerPrivate = NewActor;
        NewActor->OwnedComponents.Add(NewComponent);

        //디폴트 컴포넌트 이름 동일하게 
        for (const auto DefaultCopiedName : DefaultCopiedComponentNames)
        {
            if (DefaultCopiedName == Component->GetFName())
            {
                NewComponent->SetFName(DefaultCopiedName);
            }
        }
  
        // RootComponent 설정
        if (RootComponent == Component)
        {
            NewActor->RootComponent = static_cast<USceneComponent*>(NewComponent);
        }

        // 부모-자식 관계 저장 (USceneComponent만 해당)
        if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
        {
            OriginalToDuplicateMap.Add(SceneComp, static_cast<USceneComponent*>(NewComponent));
            if (USceneComponent* ParentComp = SceneComp->GetAttachParent())
            {
                ParentChildMap.Add(static_cast<USceneComponent*>(NewComponent), ParentComp);
            }
        }

        // 컴포넌트 초기화
        NewComponent->InitializeComponent();
    }

    // 부모-자식 관계 복원
    for (const auto& [ChildComp, OriginalParentComp] : ParentChildMap)
    {
        // 복제된 부모를 찾아 설정
        if (USceneComponent** NewParentComp = OriginalToDuplicateMap.Find(OriginalParentComp))
        {
            ChildComp->AttachToComponent(*NewParentComp);
        }
    }

    NewActor->LuaScriptComponent = NewActor->GetComponentByClass<ULuaScriptComponent>();

    NewActor->OnActorOverlapHandle.Invalidate();
    NewActor->OnActorBeginOverlapHandle.Invalidate();
    NewActor->OnActorEndOverlapHandle.Invalidate();

    return NewActor;
}

void AActor::GetProperties(TMap<FString, FString>& OutProperties) const
{
    TMap<FString, FString>& Properties = OutProperties;
    
    Properties.Add(TEXT("bTickInEditor"), bTickInEditor ? TEXT("true") : TEXT("false"));
    Properties.Add(TEXT("bUseScript"), bUseScript ? TEXT("true") : TEXT("false"));
}

void AActor::SetProperties(const TMap<FString, FString>& InProperties)
{
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("bTickInEditor"));
    if (TempStr)
    {
        bTickInEditor = *TempStr == TEXT("true");
    }
    TempStr = InProperties.Find(TEXT("bUseScript"));
    if (TempStr)
    {
        bUseScript = *TempStr == TEXT("true");
    }
}

void AActor::BeginPlay()
{
    if (bUseScript)
    {
        RegisterLuaType(FLuaScriptManager::Get().GetLua());
        BindSelfLuaProperties();
    }

    const auto CopyComponents = OwnedComponents;
    for (UActorComponent* Comp : CopyComponents)
    {  
        Comp->BeginPlay();
    }

}

void AActor::Tick(float DeltaTime)
{
    // TODO: 임시로 Actor에서 Tick 돌리기
    // TODO: 나중에 삭제를 Pending으로 하던가 해서 복사비용 줄이기
    const auto CopyComponents = OwnedComponents;

    for (UActorComponent* Comp : CopyComponents)
    {
        Comp->TickComponent(DeltaTime);
    }
}

void AActor::Destroyed()
{
    // Actor가 제거되었을 때 호출하는 EndPlay
    EndPlay(EEndPlayReason::Destroyed);
    
    TSet<UActorComponent*> Components = OwnedComponents;
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent(true);
    }
}

void AActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

    if (OnActorOverlapHandle.IsValid())
    {
        OnActorOverlap.Remove(OnActorOverlapHandle);
        OnActorOverlapHandle.Invalidate();
    }
    if (OnActorBeginOverlapHandle.IsValid())
    {
        OnActorBeginOverlap.Remove(OnActorBeginOverlapHandle);
        OnActorBeginOverlapHandle.Invalidate();
    }
    if (OnActorEndOverlapHandle.IsValid())
    {
        OnActorEndOverlap.Remove(OnActorEndOverlapHandle); 
        OnActorEndOverlapHandle.Invalidate();
    }

    for (UActorComponent* Component : GetComponents())
    {
        if (Component->HasBegunPlay())
        {
            Component->EndPlay(EndPlayReason);
        }
    }

    UninitializeComponents();
}

bool AActor::Destroy()
{
    if (!IsActorBeingDestroyed())
    {
        if (UWorld* World = GetWorld())
        {
            UE_LOG(LogLevel::Display, "Delete Component - %s", *GetName());

            World->DestroyActor(this);
            bActorIsBeingDestroyed = true;
        }
    }

    return IsActorBeingDestroyed();
}

UActorComponent* AActor::AddComponent(UClass* InClass, FName InName, bool bTryRootComponent)
{

    if (!InClass)
    {
        UE_LOG(LogLevel::Error, TEXT("UActorComponent failed: ComponentClass is null."));
        return nullptr;
    }
    
    if (InClass->IsChildOf<UActorComponent>())
    {
        UActorComponent* Component = static_cast<UActorComponent*>(FObjectFactory::ConstructObject(InClass, this, InName));

        if (!Component)
        {
            UE_LOG(LogLevel::Error, TEXT("UActorComponent failed: Class '%s' is not derived from AActor."), *InClass->GetName());
            return nullptr;
        }
        
        OwnedComponents.Add(Component);
        Component->OwnerPrivate = this;

        // 만약 SceneComponent를 상속 받았다면

        if (bTryRootComponent)
        {
            if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
            {
                if (RootComponent == nullptr)
                {
                    RootComponent = SceneComp;
                }
            // TODO: 나중에 RegisterComponent() 생기면 주석 해제
                // else
                // {
                //     SceneComp->SetupAttachment(RootComponent);
                // }
            }
        }

        // TODO: RegisterComponent() 생기면 제거
        Component->InitializeComponent();

        return Component;
    }
    
    UE_LOG(LogLevel::Error, TEXT("UActorComponent failed: ComponentClass is null."));
    return nullptr;
}

void AActor::RemoveOwnedComponent(UActorComponent* Component)
{
    OwnedComponents.Remove(Component);
}

void AActor::InitializeComponents()
{
    TSet<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->bAutoActive && !ActorComp->IsActive())
        {
            ActorComp->Activate();
        }

        ActorComp->InitializeComponent();
    }
}

void AActor::UninitializeComponents()
{
    TSet<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->HasBeenInitialized())
        {
            ActorComp->UninitializeComponent();
        }
    }
}

bool AActor::SetRootComponent(USceneComponent* NewRootComponent)
{
    if (NewRootComponent == nullptr || NewRootComponent->GetOwner() == this)
    {
        if (RootComponent != NewRootComponent)
        {
            USceneComponent* OldRootComponent = RootComponent;
            RootComponent = NewRootComponent;

            if (OldRootComponent)
            {
                OldRootComponent->SetupAttachment(RootComponent);
            }
        }
        return true;
    }
    return false;
}

FVector AActor::GetActorLocation() const
{
    return RootComponent ? RootComponent->GetRelativeLocation() : FVector(FVector::ZeroVector); 
}

FRotator AActor::GetActorRotation() const
{
    return RootComponent ? RootComponent->GetRelativeRotation() : FRotator(FVector::ZeroVector);
}

FVector AActor::GetActorScale() const
{
    return RootComponent ? RootComponent->GetRelativeScale3D() : FVector(FVector::OneVector); 
}

bool AActor::SetActorLocation(const FVector& NewLocation)
{
    if (RootComponent)
    {
       RootComponent->SetRelativeLocation(NewLocation);
        return true;
    }
    return false;
}

bool AActor::SetActorRotation(const FRotator& NewRotation)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeRotation(NewRotation);
        return true;
    }
    return false;
}

bool AActor::SetActorScale(const FVector& NewScale)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeScale3D(NewScale);
        return true;
    }
    return false;
}

void AActor::EnableInput(APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    if (!InputComponent)
    {
        InputComponent = FObjectFactory::ConstructObject<UInputComponent>(this);
    }

    PlayerController->PushInputComponent(InputComponent);

}

void AActor::DisableInput(APlayerController* PlayerController)
{
    if (!PlayerController || !InputComponent)
    {
        return;
    }

    // 입력 스택에서 제거
    PlayerController->PopInputComponent(InputComponent);
}

bool AActor::IsOverlappingActor(const AActor* Other) const
{
    for (UActorComponent* OwnedComp : OwnedComponents)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OwnedComp))
        {
            if ((PrimComp->GetOverlapInfos().Num() > 0) && PrimComp->IsOverlappingActor(Other))
            {
                return true;
            }
        }
    }
    return false;
}

void AActor::UpdateOverlaps() const
{
    for (UActorComponent* Comp : OwnedComponents)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Comp))
        {
            PrimComp->UpdateOverlaps();
        }
    }
}

void AActor::SetActorTickInEditor(bool InbInTickInEditor)
{
    bTickInEditor = InbInTickInEditor;
}

void AActor::InitLuaScriptComponent()
{
    if (LuaScriptComponent == nullptr)
    {
        LuaScriptComponent = GetComponentByFName<ULuaScriptComponent>("LuaComponent_0");
        if (LuaScriptComponent == nullptr)
        {
            LuaScriptComponent = AddComponent<ULuaScriptComponent>("LuaComponent_0");
        }
    }
}

FString AActor::GetLuaScriptPathName()
{
    return LuaScriptComponent ? LuaScriptComponent->GetScriptName() : TEXT("");
}



void AActor::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_NO_PARENT(AActor,
        "UUID", sol::property(&ThisClass::GetUUID),
        /*"ActorName", &ThisClass::GetName,*/ // FString은 넘어가지 않는 중 내부에서 사용 불가.
        "ActorLocation", sol::property(&ThisClass::GetActorLocation, &ThisClass::SetActorLocation),
        "ActorRotation", sol::property(&ThisClass::GetActorRotation, &ThisClass::SetActorRotation),
        "ActorScale", sol::property(&ThisClass::GetActorScale, &ThisClass::SetActorScale),
        "Destroy", &ThisClass::Destroy
    )
}

bool AActor::BindSelfLuaProperties()
{
    if (!LuaScriptComponent)
    {
        return false;
    }
    // LuaScript Load 실패.
    if (!LuaScriptComponent->LoadScript())
    {
        return false;
    }
    
    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    // 자기 자신 등록.
    // self에 this를 하게 되면 내부에서 임의로 Table로 바꿔버리기 때문에 self:함수() 형태의 호출이 불가능.
    // 자기 자신 객체를 따로 넘겨주어야만 AActor:GetName() 같은 함수를 실행시켜줄 수 있다.
    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName(); // FString 해결되기 전까지 임시로 Table로 전달.
    // 이 아래에서 또는 하위 클래스 함수에서 멤버 변수 등록.

    return true;
}

// Actor.cpp
void AActor::ProcessOverlaps()
{
    const auto CopyComponents = OwnedComponents; // ★ 복사

    for (UActorComponent* Component : CopyComponents)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            PrimComp->ProcessOverlaps();
        }
    }
}

