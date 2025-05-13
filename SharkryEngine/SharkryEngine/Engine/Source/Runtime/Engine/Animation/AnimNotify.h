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
    int TriggerFrame;       // AniEditorPanel UI에서 변경해주는 용도의 Frame
    float Duration;
    FName NotifyName;
    ENotifyMode NotifyMode;
    ENotifyState NotifyState;
    int TrackNum = 0;   // AnimNotify UI 쪽에서 변경할 때 몇 번째 트랙인지 판별, 1부터 유효

    // Single Notify Event에 대한 생성자
    FAnimNotifyEvent(float InTriggerTime, float InDuration, FName InNotifyName) 
        : TriggerTime(InTriggerTime), Duration(InDuration), NotifyName(InNotifyName), 
        NotifyMode(ENotifyMode::Single), NotifyState(ENotifyState::None)
    { }

    // State Notify Event에 대한 생성자
    FAnimNotifyEvent(float InTriggerTime, float InDuration, FName InNotifyName, ENotifyMode InNotifyMode, ENotifyState InNotifyState)
        : TriggerTime(InTriggerTime), Duration(InDuration), NotifyName(InNotifyName),
        NotifyMode(InNotifyMode), NotifyState(InNotifyState)
    { }

    void SetTrackNum(int InTrackNum) 
    {
        TrackNum = InTrackNum;
    }

    void UpdateTriggerTime(int FrameNum) 
    {
        TriggerTime = (float)(TriggerFrame / FrameNum);
    }

    void UpdateTriggerFrame(float PlayLength, int FrameNum) 
    {
        TriggerFrame = FMath::FloorToInt32((TriggerTime / PlayLength) * FrameNum);
    }
};
