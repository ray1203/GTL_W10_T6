#pragma once

#include "Define.h"
#include "Components/SkeletalMeshComponent.h"

class FSkeletalMeshDebugger
{
public:
    static void DrawSkeleton(const USkeletalMeshComponent* SkelMeshComp);
    static void DrawSkeletonAABBs(const USkeletalMeshComponent* SkelMeshComp);
}; 