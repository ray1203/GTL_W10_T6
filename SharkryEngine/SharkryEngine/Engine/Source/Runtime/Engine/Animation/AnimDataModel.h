#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimTypes.h"
#include "Core/Misc/FrameRate.h"

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

class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)
    
public:
    UAnimDataModel();
    ~UAnimDataModel() = default;

    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;

private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;
};

