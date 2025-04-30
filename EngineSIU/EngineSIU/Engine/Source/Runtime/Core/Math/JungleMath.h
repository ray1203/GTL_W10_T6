#pragma once
#include "Vector4.h"

class JungleMath
{
public:
    static FVector4 ConvertV3ToV4(FVector vec3);
    static FMatrix CreateModelMatrix(FVector translation, FVector rotation, FVector scale);
    static FMatrix CreateModelMatrix(FVector translation, FQuat rotation, FVector scale);
    static FMatrix CreateViewMatrix(FVector eye, FVector target, FVector up);
    static FMatrix CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane);
    static FMatrix CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane);
    static FMatrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    static FVector FVectorRotate(FVector& origin, const FVector& InRotation);
    static FVector FVectorRotate(FVector& origin, const FRotator& InRotation);
    static FMatrix CreateRotationMatrix(FVector rotation);
    static FQuat EulerToQuaternion(const FVector& eulerDegrees);
    static FVector QuaternionToEuler(const FQuat& quat);

    static FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);
    static FVector VInterpToConstant(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

    /** Interpolate quaternion from Current to Target. Scaled by angle to Target, so it has a strong start speed and ease out. */
    static FQuat QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed);
    /** Interpolate quaternion from Current to Target with constant step (in radians) */
    static FQuat QInterpConstantTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed);
};
