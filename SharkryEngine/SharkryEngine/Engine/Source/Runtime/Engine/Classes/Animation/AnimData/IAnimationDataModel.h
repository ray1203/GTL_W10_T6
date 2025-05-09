#pragma once

#include "Engine/Source/Runtime/CoreUObject/UObject/NameTypes.h"
#include "Engine/Public/Animation/AnimTypes.h"


struct FBoneAnimationTrack
{

    FName Name;                        // Bone 이름
    FRawAnimSequenceTrack InternalTrack; // 실제 애니메이션 데이터
};
