#pragma once
#include"Math/Vector.h"
#include "Math/Quat.h"

struct FBox
{
    FVector Center;
    FVector Extent;
    FQuat Rotation;

    FBox() = default;

    FBox(const FVector& InCenter, const FVector& InExtent, const FQuat& InRotation = FQuat(1,0,0,0))
        : Center(InCenter), Extent(InExtent), Rotation(InRotation)
    {
    }

    FVector GetAxisX() const { return Rotation.RotateVector(FVector(1, 0, 0)); }
    FVector GetAxisY() const { return Rotation.RotateVector(FVector(0, 1, 0)); }
    FVector GetAxisZ() const { return Rotation.RotateVector(FVector(0, 0, 1)); }

    void GetCorners(FVector OutCorners[8]) const
    {
        FVector X = GetAxisX() * Extent.X;
        FVector Y = GetAxisY() * Extent.Y;
        FVector Z = GetAxisZ() * Extent.Z;

        OutCorners[0] = Center + X + Y + Z;
        OutCorners[1] = Center + X + Y - Z;
        OutCorners[2] = Center + X - Y + Z;
        OutCorners[3] = Center + X - Y - Z;
        OutCorners[4] = Center - X + Y + Z;
        OutCorners[5] = Center - X + Y - Z;
        OutCorners[6] = Center - X - Y + Z;
        OutCorners[7] = Center - X - Y - Z;
    }
};


// Sphere
struct FSphere
{
    FVector Center;
    float Radius;

    FSphere() = default;

    FSphere(const FVector& InCenter, float InRadius)
        : Center(InCenter), Radius(InRadius) {
    }
};

struct FCapsule
{
    FVector Center;     // 중심
    FVector UpVector;   // 방향 (단위벡터)
    float HalfHeight;   // 중심부터 꼭짓점까지 거리
    float Radius;       // 반지름
    FQuat Rotation;     // 회전 (추가)

    FCapsule() = default;

    FCapsule(const FVector& InCenter, const FVector& InUp, float InHalfHeight, float InRadius, const FQuat& InRotation = FQuat(1,0,0,0))
        : Center(InCenter), UpVector(InUp), HalfHeight(InHalfHeight), Radius(InRadius), Rotation(InRotation)
    {
    }

    FVector GetPointTop() const { return Center + Rotation.RotateVector(UpVector * HalfHeight); }
    FVector GetPointBottom() const { return Center - Rotation.RotateVector(UpVector * HalfHeight); }
};
