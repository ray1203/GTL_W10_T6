#pragma once
#include "Components/Shapes/CapsuleComponent.h"
#include "GameFramework/Character.h"

class UCameraComponent;
class UInputComponent;

DECLARE_MULTICAST_DELEGATE(FOnCharacterDeath);

class APlayerCharacter : public ACharacter
{
    DECLARE_CLASS(APlayerCharacter, ACharacter)

public:
    APlayerCharacter();
    UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    /**
    * 해당 액터의 직렬화 가능한 속성들을 문자열 맵으로 반환합니다.
    * 하위 클래스는 이 함수를 재정의하여 자신만의 속성을 추가해야 합니다.
    */
    void GetProperties(TMap<FString, FString>& OutProperties) const override;

    /** 저장된 Properties 맵에서 액터의 상태를 복원합니다. */
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    // === 이동 관련 ===
    void MoveForward(float Value);
    void MoveRight(float Value);

    void HandleOverlap(AActor* OtherActor);

    // === Lua 관련 ===
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    // === Getter ===
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    float GetHealth() const { return Health; }
    float GetSpeed() const { return Speed; }
    float GetAttackDamage() const { return AttackDamage; }
    int32 GetCharacterMeshCount() const { return CharacterMeshCount; }

    // === Setter ===
    void SetHealth(float NewHealth) { Health = NewHealth; }
    void SetSpeed(float NewSpeed) { Speed = NewSpeed; }
    void SetAttackDamage(float NewAttackDamage) { AttackDamage = NewAttackDamage; }
    void AddCharacterMeshCount(int32 InCount);


    void SetCharacterMeshCount(int32 InCount);
    
    // Delegate
    FOnCharacterDeath OnDeath;

    FDelegateHandle OnPlayerDeathHandle;

protected:
    AActor* SpawnActorLua(const std::string& ClassName, const FVector& Location);

    void AddShakeModifier(float Duration, float AlphaInTime, float AlphaOutTime, float Scale);

private:
    UPROPERTY
    (TArray<UStaticMeshComponent*>, StaticMeshComponents);
    
    UPROPERTY
    (UCameraComponent*, FollowCamera, = nullptr);
    
    UPROPERTY
    (int32, CharacterMeshCount, = 0)
    
    UPROPERTY
    (float, Health, = 100.0f)

    UPROPERTY
    (float, Speed, = 1.0f)

    UPROPERTY
    (float, AttackDamage, = 10.0f)
};

