#include "AnimSingleNodeInstance.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{

}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (AnimSequence) 
    {
        FPoseContext Pose(this);

        FAnimExtractContext Extract(1.0f, false);

        AnimSequence->GetAnimationPose(Pose, Extract);

        Output.Pose = Pose.Pose;
        // Output.Curve = Pose.Curve;
    }

}
