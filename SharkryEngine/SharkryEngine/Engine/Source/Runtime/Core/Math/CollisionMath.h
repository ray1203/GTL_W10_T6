#pragma once

struct FVector;
struct FBox;
struct FSphere;
struct FCapsule;

class FCollisionMath
{
public:
    static bool IntersectBoxBox(const FBox& A, const FBox& B);
    static bool IntersectBoxSphere(const FBox& Box, const FVector& SphereCenter, float Radius);
    static bool IntersectBoxCapsule(const FBox& Box, const FCapsule& Capsule);
    static bool IntersectSphereSphere(const FSphere& A, const FSphere& B);
    static bool IntersectCapsuleSphere(const FCapsule& Capsule, const FVector& SphereCenter, float Radius);
    static bool IntersectCapsuleCapsule(const FCapsule& A, const FCapsule& B);

private:
    static FVector ClosestPointOnSegment(const FVector& A, const FVector& B, const FVector& P);
    static void ClosestPointsBetweenSegments(const FVector& P1, const FVector& Q1,
        const FVector& P2, const FVector& Q2,
        FVector& OutP, FVector& OutQ);
    static bool TestAxis(const FVector& Axis, const FBox& A, const FBox& B, const FVector& D);
    static FVector ClosestPointOnOBB(const FBox& Box, const FVector& Point);
};

