#pragma once
#include "ActorComponent.h"
#include "Math/Rotator.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const;
    virtual void DestroyComponent(bool bPromoteChildren = false) override;

    FVector GetForwardVector() const;
    FVector GetRightVector() const;
    FVector GetUpVector() const;
    
    void AddLocation(const FVector& InAddValue);
    void AddRotation(const FRotator& InAddValue);
    void AddScale(const FVector& InAddValue);

    USceneComponent* GetAttachParent() const { return AttachParent; }
    const TArray<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }

    void AttachToComponent(USceneComponent* InParent);
    void SetupAttachment(USceneComponent* InParent);
    void DetachFromComponent(USceneComponent* Target);

public:
    void SetRelativeLocation(const FVector& InLocation) { RelativeLocation = InLocation; }
    void SetRelativeRotation(const FRotator& InRotation);
    void SetRelativeRotation(const FQuat& InQuat);
    void SetRelativeScale3D(const FVector& InScale) { RelativeScale3D = InScale; }
    
    FVector GetRelativeLocation() const { return RelativeLocation; }
    FRotator GetRelativeRotation() const { return RelativeRotation; }
    FVector GetRelativeScale3D() const { return RelativeScale3D; }

    void SetWorldLocation(const FVector& InLocation);
    void SetWorldRotation(const FRotator& InRotation);
    void SetWorldRotation(const FQuat& InQuat);
    void SetWorldScale3D(const FVector& InScale);

    FVector GetWorldLocation() const;
    FRotator GetWorldRotation() const;
    FVector GetWorldScale3D() const;

    FMatrix GetScaleMatrix() const;
    FMatrix GetRotationMatrix() const;
    FMatrix GetTranslationMatrix() const;

    FMatrix GetWorldMatrix() const;
    FMatrix GetWorldRTMatrix() const;

protected:
    /** 부모 컴포넌트로부터 상대적인 위치 */
    UPROPERTY
    (FVector, RelativeLocation);

    /** 부모 컴포넌트로부터 상대적인 회전 */
    UPROPERTY
    (FRotator, RelativeRotation);

    /** 부모 컴포넌트로부터 상대적인 크기 */
    UPROPERTY
    (FVector, RelativeScale3D);


    UPROPERTY
    (USceneComponent*, AttachParent, = nullptr);

    UPROPERTY
    (TArray<USceneComponent*>, AttachChildren);
};
