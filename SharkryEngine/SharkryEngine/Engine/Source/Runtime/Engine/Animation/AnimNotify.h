#include "Engine/Source/Runtime/CoreUObject/UObject/NameTypes.h"

enum class ENotifyMode 
{
    Single,
    State
};

enum class ENotifyState 
{
    None,
    Begin,
    Tick,
    End
};


struct FAnimNotifyEvent 
{
    float TriggerTime;
    float Duration;
    FName NotifyName;
    ENotifyMode NotifyMode;
    ENotifyState NotifyState;

    // Single Notify Event에 대한 생성자
    FAnimNotifyEvent(float InTriggerTime, float InDuration, FName InNotifyName) 
        : TriggerTime(InTriggerTime), Duration(InDuration), NotifyName(InNotifyName), 
        NotifyMode(ENotifyMode::Single), NotifyState(ENotifyState::None)
    {   }

    // State Notify Event에 대한 생성자
    FAnimNotifyEvent(float InTriggerTime, float InDuration, FName InNotifyName, ENotifyMode InNotifyMode, ENotifyState InNotifyState)
        : TriggerTime(InTriggerTime), Duration(InDuration), NotifyName(InNotifyName),
        NotifyMode(InNotifyMode), NotifyState(InNotifyState)
    { }
};
