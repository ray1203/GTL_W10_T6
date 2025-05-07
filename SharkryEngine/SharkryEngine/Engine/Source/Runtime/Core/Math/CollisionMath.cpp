#include "Math/CollisionMath.h"
#include "Math/ShapeInfo.h" // FBox, FSphere, FCapsule
#include "MathUtility.h"


bool FCollisionMath::IntersectBoxBox(const FBox& A, const FBox& B)
{
    // 각 박스의 로컬 축
    FVector AxisA[3] = { A.GetAxisX(), A.GetAxisY(), A.GetAxisZ() };
    FVector AxisB[3] = { B.GetAxisX(), B.GetAxisY(), B.GetAxisZ() };

    // 두 박스 중심 벡터
    FVector D = B.Center - A.Center;

    // 15개의 축 모두 검사 (A0, A1, A2, B0, B1, B2, A0xB0, A0xB1, ..., A2xB2)
    for (int i = 0; i < 3; ++i)
    {
        if (!TestAxis(AxisA[i], A, B, D)) return false;
    }
    for (int i = 0; i < 3; ++i)
    {
        if (!TestAxis(AxisB[i], A, B, D)) return false;
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            FVector Cross = FVector::CrossProduct(AxisA[i], AxisB[j]);
            if (Cross.SquaredLength() > SMALL_NUMBER)
            {
                if (!TestAxis(Cross, A, B, D)) return false;
            }
        }
    }

    return true; // 모든 축에서 분리 안됐으면 충돌
}

bool FCollisionMath::IntersectBoxSphere(const FBox& Box, const FVector& SphereCenter, float Radius)
{
    FVector Local = SphereCenter - Box.Center;

    FVector ClosestPoint = Box.Center;
    FVector Axes[3] = { Box.GetAxisX(), Box.GetAxisY(), Box.GetAxisZ() };

    for (int i = 0; i < 3; ++i)
    {
        float Distance = FVector::DotProduct(Local, Axes[i]);
        Distance = FMath::Clamp(Distance, -Box.Extent[i], Box.Extent[i]);
        ClosestPoint += Axes[i] * Distance;
    }

    return (ClosestPoint - SphereCenter).SquaredLength() <= Radius * Radius;
}


bool FCollisionMath::IntersectBoxCapsule(const FBox& Box, const FCapsule& Capsule)
{
    FVector Top = Capsule.GetPointTop();
    FVector Bottom = Capsule.GetPointBottom();

    // 캡슐 세그먼트의 Closest Point를 OBB에 대해 찾는다
    FVector ClosestA = ClosestPointOnOBB(Box, Top);
    FVector ClosestB = ClosestPointOnOBB(Box, Bottom);

    FVector ClosestOnSegment = ClosestPointOnSegment(Top, Bottom, (ClosestA + ClosestB) * 0.5f);

    float DistSq = (ClosestPointOnOBB(Box, ClosestOnSegment) - ClosestOnSegment).SquaredLength();

    return DistSq <= Capsule.Radius * Capsule.Radius;
}


bool FCollisionMath::IntersectSphereSphere(const FSphere& A, const FSphere& B)
{
    float RadiusSum = A.Radius + B.Radius;
    return (A.Center - B.Center).SquaredLength() <= RadiusSum * RadiusSum;
}

bool FCollisionMath::IntersectCapsuleSphere(const FCapsule& Capsule, const FVector& SphereCenter, float Radius)
{
    FVector Top = Capsule.GetPointTop();
    FVector Bottom = Capsule.GetPointBottom();

    FVector Closest = ClosestPointOnSegment(Top, Bottom, SphereCenter);

    float RadiusSum = Capsule.Radius + Radius;

    return (Closest - SphereCenter).SquaredLength() <= RadiusSum * RadiusSum;
}


bool FCollisionMath::IntersectCapsuleCapsule(const FCapsule& A, const FCapsule& B)
{
    FVector P1 = A.GetPointTop();
    FVector Q1 = A.GetPointBottom();
    FVector P2 = B.GetPointTop();
    FVector Q2 = B.GetPointBottom();

    FVector ClosestP, ClosestQ;
    ClosestPointsBetweenSegments(P1, Q1, P2, Q2, ClosestP, ClosestQ);

    float RadiusSum = A.Radius + B.Radius;
    return (ClosestP - ClosestQ).SquaredLength() <= RadiusSum * RadiusSum;
}


FVector FCollisionMath::ClosestPointOnSegment(const FVector& A, const FVector& B, const FVector& P)
{
    FVector AB = B - A;
    float AB_LengthSq = AB.SquaredLength();
    if (AB_LengthSq <= SMALL_NUMBER) return A;

    float T = FVector::DotProduct(P - A, AB) / AB_LengthSq;
    T = FMath::Clamp(T, 0.0f, 1.0f);
    return A + AB * T;
}
void FCollisionMath::ClosestPointsBetweenSegments(const FVector& P1, const FVector& Q1,
    const FVector& P2, const FVector& Q2,
    FVector& OutP, FVector& OutQ)
{
    const FVector D1 = Q1 - P1;
    const FVector D2 = Q2 - P2;
    const FVector R = P1 - P2;
    const float A = D1.SquaredLength();
    const float E = D2.SquaredLength();
    const float F = FVector::DotProduct(D2, R);

    float S, T;

    if (A <= SMALL_NUMBER && E <= SMALL_NUMBER)
    {
        S = T = 0.0f;
    }
    else if (A <= SMALL_NUMBER)
    {
        S = 0.0f;
        T = FMath::Clamp(F / E, 0.0f, 1.0f);
    }
    else
    {
        const float C = FVector::DotProduct(D1, R);
        if (E <= SMALL_NUMBER)
        {
            T = 0.0f;
            S = FMath::Clamp(-C / A, 0.0f, 1.0f);
        }
        else
        {
            const float B = FVector::DotProduct(D1, D2);
            const float Denom = A * E - B * B;
            if (Denom != 0.0f)
            {
                S = FMath::Clamp((B * F - C * E) / Denom, 0.0f, 1.0f);
            }
            else S = 0.0f;

            T = (B * S + F) / E;

            if (T < 0.0f) { T = 0.0f; S = FMath::Clamp(-C / A, 0.0f, 1.0f); }
            else if (T > 1.0f) { T = 1.0f; S = FMath::Clamp((B - C) / A, 0.0f, 1.0f); }
        }
    }

    OutP = P1 + D1 * S;
    OutQ = P2 + D2 * T;
}

bool FCollisionMath::TestAxis(const FVector& Axis, const FBox& A, const FBox& B, const FVector& D)
{
    float ProjectA =
        A.Extent.X * FMath::Abs(FVector::DotProduct(Axis, A.GetAxisX())) +
        A.Extent.Y * FMath::Abs(FVector::DotProduct(Axis, A.GetAxisY())) +
        A.Extent.Z * FMath::Abs(FVector::DotProduct(Axis, A.GetAxisZ()));

    float ProjectB =
        B.Extent.X * FMath::Abs(FVector::DotProduct(Axis, B.GetAxisX())) +
        B.Extent.Y * FMath::Abs(FVector::DotProduct(Axis, B.GetAxisY())) +
        B.Extent.Z * FMath::Abs(FVector::DotProduct(Axis, B.GetAxisZ()));

    float Distance = FMath::Abs(FVector::DotProduct(D, Axis));

    return Distance <= (ProjectA + ProjectB);
}

FVector FCollisionMath::ClosestPointOnOBB(const FBox& Box, const FVector& Point)
{
    FVector Local = Point - Box.Center;
    FVector Result = Box.Center;

    FVector Axis[3] = { Box.GetAxisX(), Box.GetAxisY(), Box.GetAxisZ() };
    for (int i = 0; i < 3; ++i)
    {
        float Distance = FVector::DotProduct(Local, Axis[i]);
        Distance = FMath::Clamp(Distance, -Box.Extent[i], Box.Extent[i]);
        Result += Axis[i] * Distance;
    }
    return Result;
}

