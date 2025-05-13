#include "AnimationRuntimeUtils.h"
#include "Math/JungleMath.h"
#include "AnimInstance.h"

namespace FAnimationRuntime
{
    void BlendTwoCompactPoses(const FCompactPose& InPoseA, const FCompactPose& InPoseB, float InBlendAlpha, FCompactPose& OutBlendedPose)
    {
        const int32 NumBonesA = InPoseA.BoneTransforms.Num();
        const int32 NumBonesB = InPoseB.BoneTransforms.Num();

        if (0 == NumBonesA || 0 == NumBonesB)
        {
            // 포즈 중 하나라도 비어있으면 아무것도 하지 않음
            return;
        }
        if (NumBonesA != NumBonesB)
        {
            // 두 포즈의 뼈 수가 다르면 오류 처리
            return;
        }

        OutBlendedPose.BoneTransforms.SetNum(NumBonesA);
        const float Alpha = FMath::Clamp(InBlendAlpha, 0.0f, 1.0f);

        for (int32 BoneIndex = 0; BoneIndex < NumBonesA; ++BoneIndex)
        {
            const FMatrix& TransformA = InPoseA.BoneTransforms[BoneIndex];
            const FMatrix& TransformB = InPoseB.BoneTransforms[BoneIndex];
            
            // 행렬 분해
            FVector TranslationA = JungleMath::DecomposeTranslation(TransformA);
            FQuat RotationA = JungleMath::DecomposeRotation(TransformA);
            FVector ScaleA = JungleMath::DecomposeScale(TransformA);

            FVector TranslationB = JungleMath::DecomposeTranslation(TransformB);
            FQuat RotationB = JungleMath::DecomposeRotation(TransformB);
            FVector ScaleB = JungleMath::DecomposeScale(TransformB);

            // 요소별 보간
            FVector BlendedTranslation = FMath::Lerp(TranslationA, TranslationB, Alpha);
            FQuat BlendedRotation = FQuat::Slerp(RotationA, RotationB, Alpha);
            FVector BlendedScale = FMath::Lerp(ScaleA, ScaleB, Alpha);

            // 최종 행렬 재조합
            OutBlendedPose.BoneTransforms[BoneIndex] = JungleMath::CreateModelMatrix(BlendedTranslation, BlendedRotation, BlendedScale);
        }
    }
}
