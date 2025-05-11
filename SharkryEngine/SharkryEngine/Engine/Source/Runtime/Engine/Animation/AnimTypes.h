#pragma once
#include "UObject/NameTypes.h"
#include "Core/Math/Quat.h"
#include "Core/Math/Matrix.h"
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


struct FAnimExtractContext
{
    float CurrentTime;
    bool  bLooping;
    bool  bExtractRootMotion;
    bool  bForceUseRawData;
    // …
};

// 1) 본 계층 정보: 본 개수, 부모 인덱스, 커브 개수 등
struct FBoneContainer
{
    TArray<FName> ParentNames;

    // 커브 채널 개수가 있다면 여기에 추가
    int32 NumCurves = 0;

    int32 GetNumBones() const { return ParentNames.Num(); }
    int32 GetNumCurves() const { return NumCurves; }
};

// 2) CompactPose: 본마다 컴포넌트 스페이스 트랜스폼을 담는 버퍼
struct FCompactPose
{
    // BoneTransforms[i] 는 i번 본의 컴포넌트 스페이스 트랜스폼
    TArray<FMatrix> BoneMatrices;

    FCompactPose() = default;
    explicit FCompactPose(int32 NumBones)
    {
        BoneMatrices.SetNum(NumBones);
        // 기본값은 단위 행렬(Identity)로 채워 두는 게 편리합니다.
        for (auto& T : BoneMatrices)
            T = FMatrix::Identity;
    }
};

// 3) BlendedCurve: 애니메이션 커브(파라미터) 값을 담는 버퍼
struct FBlendedCurve
{
    // CurveValues[i] 는 i번 커브 채널의 현재 값
    TArray<float> CurveValues;

    FBlendedCurve() = default;
    explicit FBlendedCurve(int32 NumCurves)
    {
        CurveValues.Init(0.f, NumCurves);
    }
};

class UAnimInstance;
// 4) FPoseContext: 실제 Pose/Curve + 본 계층 참조를 한 번에 묶는 구조체
struct FPoseContext
{
    FCompactPose       Pose;
    FBlendedCurve      Curve;
    const FBoneContainer& BoneContainer;

    // 생성자에서 BoneContainer 레퍼런스를 받아,
    // Pose와 Curve를 적절한 크기로 초기화합니다.
    FPoseContext(const FBoneContainer& InBoneContainer);

    FPoseContext(const UAnimInstance* AnimInst);
};

inline void SamplePose(
    const FRawAnimSequenceTrack& Track,
    FMatrix& OutTransform,
    float Time,
    const FFrameRate& FrameRate)
{
    // 프레임 단위로 변환:  예) FrameRate = 30fps, Time=1.5초 → FrameNumber=45
    double FrameNumber = double(Time) * FrameRate.AsDecimal();
    int32 NumPos = Track.PosKeys.Num();
    int32 NumRot = Track.RotKeys.Num();
    int32 NumScale = Track.ScaleKeys.Num();

    FMatrix TranslationMatrix;
    // 위치 보간
    if (NumPos > 0)
    {
        int32 I0 = FMath::Clamp(int32(FMath::FloorToDouble(FrameNumber)), 0, NumPos - 1);
        int32 I1 = FMath::Clamp(I0 + 1, 0, NumPos - 1);
        float Alpha = float(FMath::Frac(FrameNumber));
        const FVector& P0 = Track.PosKeys[I0];
        const FVector& P1 = Track.PosKeys[I1];
        TranslationMatrix = FMatrix::GetTranslationMatrix(FMath::Lerp(P0, P1, Alpha));
    }
    else
    {
        TranslationMatrix = FMatrix::GetTranslationMatrix(FVector::ZeroVector);
    }

    FMatrix RotationMatrix;
    // 회전 보간
    if (NumRot > 0)
    {
        int32 I0 = FMath::Clamp(int32(FMath::FloorToDouble(FrameNumber)), 0, NumRot - 1);
        int32 I1 = FMath::Clamp(I0 + 1, 0, NumRot - 1);
        float Alpha = float(FMath::Frac(FrameNumber));
        const FQuat& Q0 = Track.RotKeys[I0];
        const FQuat& Q1 = Track.RotKeys[I1];
        RotationMatrix = FMatrix::GetRotationMatrix(FQuat::Slerp(Q0, Q1, Alpha));
    }
    else
    {
        RotationMatrix = FMatrix::GetRotationMatrix(FQuat());
    }

    FMatrix ScaleMatrix;
    // 스케일 보간
    if (NumScale > 0)
    {
        int32 I0 = FMath::Clamp(int32(FMath::FloorToDouble(FrameNumber)), 0, NumScale - 1);
        int32 I1 = FMath::Clamp(I0 + 1, 0, NumScale - 1);
        float Alpha = float(FMath::Frac(FrameNumber));
        const FVector& S0 = Track.ScaleKeys[I0];
        const FVector& S1 = Track.ScaleKeys[I1];
        ScaleMatrix = FMatrix::GetScaleMatrix(FMath::Lerp(S0, S1, Alpha));
    }
    else
    {
        ScaleMatrix = FMatrix::GetScaleMatrix(FVector(1.f));
    }

    OutTransform = ScaleMatrix * RotationMatrix * TranslationMatrix;
}

/**
 * 단일 커브 채널을 시간(Time: 초)에 따라 보간한 값을 반환합니다.
 */
inline float EvaluateCurveAtTime(const FAnimationCurveChannel& Channel, float Time)
{
    const auto& Keys = Channel.Keys;
    int32 Num = Keys.Num();
    if (Num == 0)
        return 0.f;

    // 시간 범위 밖이면 첫/마지막 값
    if (Time <= Keys[0].Time)           return float(Keys[0].Value);
    if (Time >= Keys[Num - 1].Time)       return float(Keys[Num - 1].Value);

    // 키 사이 찾기
    for (int32 i = 0; i < Num - 1; ++i)
    {
        const auto& K0 = Keys[i];
        const auto& K1 = Keys[i + 1];
        if (Time < K1.Time)
        {
            double Delta = K1.Time - K0.Time;
            if (Delta <= 0.0) return float(K0.Value);

            float Alpha = float((Time - K0.Time) / Delta);
            switch (K0.InterpMode)
            {
            case RCIM_Linear:
                return FMath::Lerp(float(K0.Value), float(K1.Value), Alpha);

            case RCIM_Constant:
                return float(K0.Value);

            case RCIM_Cubic:
            {
                // Hermite interpolation
                float T = Alpha;
                float T2 = T * T, T3 = T2 * T;
                float H00 = 2 * T3 - 3 * T2 + 1;
                float H10 = T3 - 2 * T2 + T;
                float H01 = -2 * T3 + 3 * T2;
                float H11 = T3 - T2;
                float M0 = float(K0.LeaveTangent * Delta);
                float M1 = float(K1.ArriveTangent * Delta);
                return H00 * float(K0.Value)
                    + H10 * M0
                    + H01 * float(K1.Value)
                    + H11 * M1;
            }
            }
        }
    }
    // 혹시 빠졌으면 마지막 값 반환
    return float(Keys[Num - 1].Value);
}
