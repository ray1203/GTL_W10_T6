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
    int TriggerFrame;       // AnimEditorPanel UI에서 변경해주는 용도의 Frame
    float Duration;
    int TriggerEndFrame;    // AnimEditorPanel UI에서 변경해주는 용도의 Frame
    bool isTriggerEndClicked;
    FName NotifyName;
    ENotifyMode NotifyMode;
    ENotifyState NotifyState;
    int TrackNum = 1;   // AnimNotify UI 쪽에서 변경할 때 몇 번째 트랙인지 판별, 1부터 유효

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

    void UpdateTriggerTime(float PlayLength, int FrameNum)
    {
        if (!isTriggerEndClicked && TriggerFrame > TriggerEndFrame) 
        {
            TriggerEndFrame = TriggerFrame;
        }

        TriggerTime = ((float)TriggerFrame / FrameNum) * PlayLength;
    }

    void UpdateTriggerFrame(float PlayLength, int FrameNum) 
    {
        TriggerFrame = FMath::FloorToInt32((TriggerTime / PlayLength) * FrameNum);
    }

    void UpdateTriggerEndTime(float PlayLength, int FrameNum) 
    {
        float TriggerEndTime = ((float)TriggerEndFrame / FrameNum) * PlayLength;
        if (isTriggerEndClicked && TriggerTime > TriggerEndTime)
        {
            TriggerTime = TriggerEndTime;
            Duration = 0.0f;
            return;
        } 
        Duration = TriggerEndTime - TriggerTime;
    }

    void UpdateTriggerEndFrame(float PlayLength, int FrameNum) 
    {
        TriggerEndFrame = FMath::FloorToInt32(((TriggerTime + Duration) / PlayLength) * FrameNum);
    }
};
