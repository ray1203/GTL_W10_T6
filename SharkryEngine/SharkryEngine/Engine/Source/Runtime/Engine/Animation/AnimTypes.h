#pragma once
#include "UObject/NameTypes.h"
#include "Core/Math/Quat.h"
#include "Core/Math/Vector.h"
#include "Core/Container/Array.h"
#include "Core/Misc/FrameRate.h"

struct FAnimNotifyEvent
{
    float TriggerTime;
    float Duration;
    FName NotifyName;
};

enum EInterpMode : uint8
{
    RCIM_Cubic,
    RCIM_Linear,
    RCIM_Constant
};

struct FAnimationCurveKey
{
    double Time;                       // 샘플링 시간(초 또는 프레임)
    double Value;                      // 커브 값
    EInterpMode InterpMode;            // 보간 모드 (Linear, Cubic, Constant 등)
    double ArriveTangent;              // 왼쪽(입력) 탄젠트
    double LeaveTangent;               // 오른쪽(출력) 탄젠트
};

// FBX 에서 하나의 채널(FbxAnimCurve)과 대응되는 커브
struct FAnimationCurveChannel
{
    FName PropertyName; // 예: "BoneName.Translation.X" 또는 "MorphTarget.Weight"
    TArray<FAnimationCurveKey> Keys;  // 키프레임 배열
};

// 전체 애니메이션 커브 데이터 컨테이너
struct FAnimationCurveData
{
    /**
     * FBX 에서 읽은 모든 커브 채널을 이 배열에 담습니다.
     * 각 채널은 PropertyName 과 키프레임(시간·값·모드·탄젠트)을 가집니다.
     */
    TArray<FAnimationCurveChannel> Channels;
};

struct FRawAnimSequenceTrack
{
    TArray<FVector> PosKeys;   // 위치 키프레임
    TArray<FQuat>   RotKeys;   // 회전 키프레임 (Quaternion)
    TArray<FVector> ScaleKeys; // 스케일 키프레임
};

struct FBoneAnimationTrack
{
    FName Name;                        // Bone 이름
    FRawAnimSequenceTrack InternalTrack; // 실제 애니메이션 데이터
};

struct FSkeletalAnimationSequence
{
    /** 클립 식별 이름 */
    FString Name;

    /** 전체 재생 길이(초) */
    float SequenceLength = 0.0f;

    /** 키프레임 간 시간 계산용 프레임레이트 */
    FFrameRate FrameRate;

    /** 본별 키 트랙 */
    TArray<FBoneAnimationTrack> BoneTracks;

    /** 모핑·파라미터 커브 데이터 */
    FAnimationCurveData CurveData;
};
