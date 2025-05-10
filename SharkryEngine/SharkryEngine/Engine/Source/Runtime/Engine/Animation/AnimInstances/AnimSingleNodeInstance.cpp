#include "AnimSingleNodeInstance.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{

}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (bPlaying && AnimSequence)
    {
        // 재생 속도(PlayRate)를 곱해서야 제대로 속도 조절이 됩니다.
        CurrentTime += DeltaSeconds * PlayRate;

        if (bLooping)
        {
            // 시퀀스 길이를 넘어가면 맨 앞으로 되돌리기
            CurrentTime = FMath::Fmod(CurrentTime, AnimSequence->GetPlayLength());
        }
        else
        {
            // 한 번만 재생할 땐 끝 시간을 넘지 않도록 고정
            CurrentTime = FMath::Clamp(CurrentTime, 0.f, AnimSequence->GetPlayLength());
        }
    }

    FPoseContext Pose(this);

    FAnimExtractContext Extract(CurrentTime, false);

    AnimSequence->GetAnimationPose(Pose, Extract);

    Output.Pose = Pose.Pose;
    // Output.Curve = Pose.Curve;
    

}
