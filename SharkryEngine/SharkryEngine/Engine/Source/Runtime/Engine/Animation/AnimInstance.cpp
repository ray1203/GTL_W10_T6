#include "AnimInstance.h"

UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
}

void UAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    // (생성 직후 Initialize 시점에만) NativeInitializeAnimation 호출
    NativeInitializeAnimation();

    // **여기서** 서브클래스가 오버라이드한 NativeUpdateAnimation 호출
    NativeUpdateAnimation(DeltaSeconds);
}

void UAnimInstance::NativeInitializeAnimation()
{
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
}
