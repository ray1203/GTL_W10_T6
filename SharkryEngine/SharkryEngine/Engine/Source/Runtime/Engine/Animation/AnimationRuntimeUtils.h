#pragma once
#include "Animation/AnimTypes.h"

namespace FAnimationRuntime
{
    /**
      * 두 개의 FCompactPose를 선형 보간합니다.
      * @param InPoseA 첫 번째 입력 포즈.
      * @param InPoseB 두 번째 입력 포즈.
      * @param InBlendAlpha 블렌딩 가중치 (0.0 = 100% InPoseA, 1.0 = 100% InPoseB).
      * @param OutBlendedPose 블렌딩된 결과 포즈가 저장될 곳.
      * @note 실제 행렬 블렌딩은 Translation(Lerp), Rotation(Slerp), Scale(Lerp)으로 수행해야 합니다.
      */
    void BlendTwoCompactPoses(
        const FCompactPose& InPoseA,
        const FCompactPose& InPoseB,
        float InBlendAlpha,
        FCompactPose& OutBlendedPose
    );

}
