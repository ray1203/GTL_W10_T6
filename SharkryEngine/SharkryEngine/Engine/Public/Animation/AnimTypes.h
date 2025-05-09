#pragma once

#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Source/Runtime/Core/Math/Vector.h"
#include "Engine/Source/Runtime/Core/Math/Quat.h"

struct FRawAnimSequenceTrack
{
    TArray<FVector> PosKeys;    // 위치 키 프레임
    TArray<FQuat> RotKeys;      // 회전 키 프레임 (Quaternion)
    TArray<FVector> ScaleKeys; // 스케일 키프레임

};

struct FBoneAnimationTrack 
{
    FName Name;     // Bone 이름
    FRawAnimSequenceTrack InternalTrack; // 실제 애니메이션 데이터
};


struct FAnimNotifyEvent
{
    float TriggerTime;
    float Duration;
    FName NotifyName;
};
