#pragma once
#include "Components/SceneComponent.h"
#include "Container/Set.h"
#include "Engine/EngineTypes.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"
#include "Delegates/DelegateCombination.h"

class UActorComponent;
class UInputComponent;
class APlayerController;
class ULuaScriptComponent;

namespace sol
{
	class state;
}
// Actor가 다른 Actor와 충돌했을 때 호출될 Delegate
DECLARE_MULTICAST_DELEGATE_OneParam(FActorHitSignature, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FActorBeginOverlapSignature, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FActorEndOverlapSignature, AActor*);

class UActorComponent;

class AActor : public UObject
{
    DECLARE_CLASS(AActor, UObject)

public:
    AActor() = default;

    // SpawnActor 내부에서 Actor 생성 이후 호출될 함수.
    // 생성 로직 단계에서 계층 구조에 종속되는 초기화를 대신 해주는 초기화 함수.
    virtual void PostSpawnInitialize();
    virtual UObject* Duplicate(UObject* InOuter) override;

    /**
    * 해당 액터의 직렬화 가능한 속성들을 문자열 맵으로 반환합니다.
    * 하위 클래스는 이 함수를 재정의하여 자신만의 속성을 추가해야 합니다.
    */
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const;

    /** 저장된 Properties 맵에서 액터의 상태를 복원합니다. */
    virtual void SetProperties(const TMap<FString, FString>& InProperties);

    /** Actor가 게임에 배치되거나 스폰될 때 호출됩니다. */
    virtual void BeginPlay();

    /** 매 Tick마다 호출됩니다. */
    virtual void Tick(float DeltaTime);

    /** Actor가 제거될 때 호출됩니다. */
    virtual void Destroyed();

    /**
     * 액터가 게임 플레이를 종료할 때 호출되는 함수입니다.
     *
     * @param EndPlayReason EndPlay가 호출된 이유를 나타내는 열거형 값
     * @note Destroyed와는 다른점은, EndPlay는 레벨 전환, 게임 종료, 또는 Destroy() 호출 시 항상 실행됩니다.
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
    /** 이 Actor를 제거합니다. */
    virtual bool Destroy();

    /** 현재 Actor가 제거중인지 여부를 반환합니다. */
    bool IsActorBeingDestroyed() const
    {
        return bActorIsBeingDestroyed;
    }

    /**
     * Actor에 컴포넌트를 새로 추가합니다.
     * @tparam T UActorComponent를 상속받은 Component
     * @return 생성된 Component
     */
    template <typename T>
        requires std::derived_from<T, UActorComponent>
    T* AddComponent(FName InName = NAME_None);
    UActorComponent* AddComponent(UClass* InClass, FName InName = NAME_None, bool bTryRootComponent = true);


    /** Actor가 가지고 있는 Component를 제거합니다. */
    void RemoveOwnedComponent(UActorComponent* Component);

    /** Actor가 가지고 있는 모든 컴포넌트를 가져옵니다. */
    const TSet<UActorComponent*>& GetComponents() const { return OwnedComponents; }

    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetComponentByClass();

    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetComponentByFName(FName InName);

    void InitializeComponents();
    void UninitializeComponents();

public:
    USceneComponent* GetRootComponent() const { return RootComponent; }
    bool SetRootComponent(USceneComponent* NewRootComponent);

    AActor* GetOwner() const { return Owner; }
    void SetOwner(AActor* NewOwner) { Owner = NewOwner; }

public:
    FVector GetActorLocation() const;
    FRotator GetActorRotation() const;
    FVector GetActorScale() const;

    FVector GetActorForwardVector() const { return RootComponent ? RootComponent->GetForwardVector() : FVector::ForwardVector; }
    FVector GetActorRightVector() const { return RootComponent ? RootComponent->GetRightVector() : FVector::RightVector; }
    FVector GetActorUpVector() const { return RootComponent ? RootComponent->GetUpVector() : FVector::UpVector; }

    bool SetActorLocation(const FVector& NewLocation);
    bool SetActorRotation(const FRotator& NewRotation);
    bool SetActorScale(const FVector& NewScale);

public:
    bool IsOverlappingActor(const AActor* Other) const;
    void UpdateOverlaps() const;

public:
    // 충돌시 호출되는 Delegate
    FActorHitSignature OnActorOverlap;
    FActorBeginOverlapSignature OnActorBeginOverlap;
    FActorEndOverlapSignature OnActorEndOverlap;

    FDelegateHandle OnActorOverlapHandle;
    FDelegateHandle OnActorBeginOverlapHandle;
    FDelegateHandle OnActorEndOverlapHandle;


    void ProcessOverlaps();

    virtual void EnableInput(APlayerController* PlayerController);
    virtual void DisableInput(APlayerController* PlayerController);

protected:
    UPROPERTY
    (USceneComponent*, RootComponent, = nullptr)

    UPROPERTY
    (UInputComponent*, InputComponent, = nullptr)

private:
    /** 이 Actor를 소유하고 있는 다른 Actor의 정보 */
    UPROPERTY
    (AActor*, Owner, = nullptr)

    /** 본인이 소유하고 있는 컴포넌트들의 정보 */
    TSet<UActorComponent*> OwnedComponents;


    /** 현재 Actor가 삭제 처리중인지 여부 */
    uint8 bActorIsBeingDestroyed : 1 = false;

    TArray<AActor*> PendingDestroyActors;



#if 1 // TODO: WITH_EDITOR 추가
public:
    /** Actor의 기본 Label을 가져옵니다. */
    FString GetDefaultActorLabel() const;

    /** Actor의 Label을 가져옵니다. */
    FString GetActorLabel() const;

    /** Actor의 Label을 설정합니다. */
    void SetActorLabel(const FString& NewActorLabel, bool bUUID = true);

private:
    /** 에디터상에 보이는 Actor의 이름 */
    UPROPERTY
    (FString, ActorLabel)
#endif

public:
    bool IsActorTickInEditor() const { return bTickInEditor; }
    void SetActorTickInEditor(bool InbInTickInEditor);

private:
    bool bTickInEditor = false;


public: // Lua Script.
	// 자기 자신이 가진 정보들 Lua에 등록.
	void InitLuaScriptComponent();
	FString GetLuaScriptPathName();
	virtual void RegisterLuaType(sol::state& Lua); // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties(); // LuaEnv에서 사용할 멤버 변수 등록 함수.

	bool bUseScript = true;
protected:
	ULuaScriptComponent* LuaScriptComponent = nullptr;

};

template <typename T>
    requires std::derived_from<T, UActorComponent>
T* AActor::AddComponent(FName InName)
{
    return Cast<T>(AddComponent(T::StaticClass(), InName));
}

template <typename T> requires std::derived_from<T, UActorComponent>
T* AActor::GetComponentByClass()
{
    for (UActorComponent* Component : OwnedComponents)
    {
        if (T* CastedComponent = Cast<T>(Component))
        {
            return CastedComponent;
        }
    }
    return nullptr;
}

template<typename T>
   requires std::derived_from<T, UActorComponent>
T* AActor::GetComponentByFName(FName InName)
{
   for (UActorComponent* Component : OwnedComponents)
   {
       if (Component->GetFName() == InName)
       {
           if (T* CastedComponent = Cast<T>(Component))
           {
               return CastedComponent;
           }
       }
   }
   return nullptr;
}
