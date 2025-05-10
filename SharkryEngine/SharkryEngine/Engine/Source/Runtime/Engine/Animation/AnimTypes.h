#pragma once
#include "UObject/NameTypes.h"
#include "Core/Math/Quat.h"
#include "Core/Math/Vector.h"
#include "Core/Container/Array.h"
// 작업 하다보면 아래부분에 재정의 문제 생길 수 있음 유의할것
#include "Engine/Animation/AnimInstance.h"

struct FAnimNotifyEvent
{
    float TriggerTime;
    float Duration;
    FName NotifyName;
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


struct FCompactPose     // FCompactPose: 실제 로컬 포즈 데이터를 담는 컨테이너
{
    // 'Compact' 순서대로 들어 있는 각 뼈의 로컬 변환
    TArray<FMatrix> BoneTransforms;
};


struct FPoseContext 
{
    // TODO 구현하다가 아래 필요한 부분에 대해서 적용하기
    UAnimInstance* AnimInstance;   // 언리얼은 FAnimInstanceProxy
    FCompactPose Pose;
    //FBlendedCurve Curve;
    //FStackCustomAttribues Attributes;
    bool bIsAdditivePose;

    FPoseContext()
        :AnimInstance(nullptr), bIsAdditivePose(false)
    {}

    FPoseContext(UAnimInstance* InAnimInstance, bool bInExpectsAdditive = false) 
        :AnimInstance(InAnimInstance), bIsAdditivePose(bInExpectsAdditive)
    { 
    }

    void ResetToRefPose(const TArray<FMatrix>& RefPoses) 
    {
        Pose.BoneTransforms.Empty();

        for (const FMatrix& RefPose : RefPoses)
        {
            Pose.BoneTransforms.Add(RefPose);
        }
    }

};

struct FAnimExtractContext 
{
    double CurrentTime;
    bool bExtractRootMotion;
    //FDeltaTimeRecord DeltaTimeRecord;           // 루트 모션 계산용 ΔTime 범위
    bool bLooping;
    TArray<bool> BonesRequired; // 필요한 뼈만 샘플링하기 위한 플래그 배열
    //TOptional<EAnimInterpolationType> InterpolationOverride;     // 보간 모드 오버라이드 (예: Cubic vs. Linear)
    bool bExtractWithRootMotionProvider; // (실험적) 루트 모션 공급자 사용 여부
    bool bIgnoreRootLock;           // 루트 잠금 무시 여부
    //TArray<FPoseCurve> PoseCurves;                // 포즈 에셋 추출 시 커브값 배열

    FAnimExtractContext(float InCurrentTime, bool InbLooping) 
        :CurrentTime(InCurrentTime), bLooping(InbLooping)
    {}
};
