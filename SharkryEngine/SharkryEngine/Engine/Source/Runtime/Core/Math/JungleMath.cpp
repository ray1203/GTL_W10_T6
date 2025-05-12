#include "Math/JungleMath.h"

#include <DirectXMath.h>

#include "Matrix.h"
#include "Quat.h"
#include "Rotator.h"


using namespace DirectX;


FVector4 JungleMath::ConvertV3ToV4(FVector vec3)
{
	FVector4 newVec4;
	newVec4.X = vec3.X;
	newVec4.Y = vec3.Y;
	newVec4.Z = vec3.Z;
	return newVec4;
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FVector rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);

    FMatrix Rotation = FMatrix::CreateRotationMatrix(rotation.X, rotation.Y, rotation.Z);
    //FMatrix Rotation = JungleMath::EulerToQuaternion(rotation).ToMatrix();

    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FQuat rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);
    FMatrix Rotation = rotation.ToMatrix();
    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}
FMatrix JungleMath::CreateViewMatrix(FVector eye, FVector target, FVector up)
{
    FVector zAxis = (target - eye).GetSafeNormal();  // DirectX는 LH이므로 -z가 아니라 +z 사용
    FVector xAxis = (up.Cross(zAxis)).GetSafeNormal();
    FVector yAxis = zAxis.Cross(xAxis);

    FMatrix View;
    View.M[0][0] = xAxis.X; View.M[0][1] = yAxis.X; View.M[0][2] = zAxis.X; View.M[0][3] = 0;
    View.M[1][0] = xAxis.Y; View.M[1][1] = yAxis.Y; View.M[1][2] = zAxis.Y; View.M[1][3] = 0;
    View.M[2][0] = xAxis.Z; View.M[2][1] = yAxis.Z; View.M[2][2] = zAxis.Z; View.M[2][3] = 0;
    View.M[3][0] = -xAxis.Dot(eye);
    View.M[3][1] = -yAxis.Dot(eye);
    View.M[3][2] = -zAxis.Dot(eye);
    View.M[3][3] = 1;

    return View;
}

FMatrix JungleMath::CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane)
{
    float tanHalfFOV = tan(fov / 2.0f);
    float depth = farPlane - nearPlane;

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / (aspect * tanHalfFOV);
    Projection.M[1][1] = 1.0f / tanHalfFOV;
    Projection.M[2][2] = farPlane / depth;
    Projection.M[2][3] = 1.0f;
    Projection.M[3][2] = -(nearPlane * farPlane) / depth;
    Projection.M[3][3] = 0.0f;  

    return Projection;
}

FMatrix JungleMath::CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane)
{
    float r = width * 0.5f;
    float t = height * 0.5f;
    float invDepth = 1.0f / (farPlane - nearPlane);

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / r;
    Projection.M[1][1] = 1.0f / t;
    Projection.M[2][2] = invDepth;
    Projection.M[3][2] = -nearPlane * invDepth;
    Projection.M[3][3] = 1.0f;

    return Projection;
}

/**
 * Create an off-center orthographic projection.
 * @param left     좌표계의 왼쪽(Xmin)
 * @param right    좌표계의 오른쪽(Xmax)
 * @param bottom   좌표계의 아래(Ymin)
 * @param top      좌표계의 위(Ymax)
 * @param nearPlane  near plane 거리
 * @param farPlane   far plane 거리
 */
FMatrix JungleMath::CreateOrthographicOffCenter( float left, float right, float bottom, float top,
    float nearPlane, float farPlane)
{
    float width = right - left;
    float height = top - bottom;
    float invWidth = 2.0f / width;       // = 1 / (width*0.5f)
    float invHeight = 2.0f / height;      // = 1 / (height*0.5f)
    float invDepth = 1.0f / (farPlane - nearPlane);

    FMatrix Projection = {};

    Projection.M[0][0] = invWidth;            // X 스케일
    Projection.M[1][1] = invHeight;           // Y 스케일
    Projection.M[2][2] = invDepth;            // Z 스케일 (0~1)
    Projection.M[3][0] = -(right + left) / width;   // X 오프셋
    Projection.M[3][1] = -(top + bottom) / height; // Y 오프셋
    Projection.M[3][2] = -nearPlane * invDepth;      // Z 오프셋
    Projection.M[3][3] = 1.0f;

    return Projection;
}

FVector JungleMath::FVectorRotate(const FVector& origin, const FVector& InRotation)
{
    FQuat quaternion = JungleMath::EulerToQuaternion(InRotation);
    // 쿼터니언을 이용해 벡터 회전 적용
    return quaternion.RotateVector(origin);
}
FQuat JungleMath::EulerToQuaternion(const FVector& eulerDegrees)
{
    float yaw = FMath::DegreesToRadians(eulerDegrees.Z);   // Z축 Yaw
    float pitch = FMath::DegreesToRadians(eulerDegrees.Y); // Y축 Pitch
    float roll = FMath::DegreesToRadians(eulerDegrees.X);  // X축 Roll

    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;

    float cosYaw = cos(halfYaw);
    float sinYaw = sin(halfYaw);
    float cosPitch = cos(halfPitch);
    float sinPitch = sin(halfPitch);
    float cosRoll = cos(halfRoll);
    float sinRoll = sin(halfRoll);

    FQuat quat;
    quat.W = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    quat.X = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    quat.Y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    quat.Z = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

    quat = quat.GetSafeNormal();
    return quat;
}
FVector JungleMath::QuaternionToEuler(const FQuat& quat)
{
    FVector euler;

    // 쿼터니언 정규화
    FQuat q = quat.GetSafeNormal();

    // Yaw (Z 축 회전)
    float sinYaw = 2.0f * (q.W * q.Z + q.X * q.Y);
    float cosYaw = 1.0f - 2.0f * (q.Y * q.Y + q.Z * q.Z);
    euler.Z = FMath::RadiansToDegrees(atan2(sinYaw, cosYaw));

    // Pitch (Y 축 회전, 짐벌락 방지)
    float sinPitch = 2.0f * (q.W * q.Y - q.Z * q.X);
    if (fabs(sinPitch) >= 1.0f)
    {
        euler.Y = FMath::RadiansToDegrees(copysign(PI / 2, sinPitch)); // 🔥 Gimbal Lock 방지
    }
    else
    {
        euler.Y = FMath::RadiansToDegrees(asin(sinPitch));
    }

    // Roll (X 축 회전)
    float sinRoll = 2.0f * (q.W * q.X + q.Y * q.Z);
    float cosRoll = 1.0f - 2.0f * (q.X * q.X + q.Y * q.Y);
    euler.X = FMath::RadiansToDegrees(atan2(sinRoll, cosRoll));
    return euler;
}
FVector JungleMath::FVectorRotate(FVector& origin, const FRotator& InRotation)
{
    return InRotation.ToQuaternion().RotateVector(origin);
}

FMatrix JungleMath::CreateRotationMatrix(FVector rotation)
{
    XMVECTOR quatX = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), FMath::DegreesToRadians(rotation.X));
    XMVECTOR quatY = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), FMath::DegreesToRadians(rotation.Y));
    XMVECTOR quatZ = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), FMath::DegreesToRadians(rotation.Z));

    XMVECTOR rotationQuat = XMQuaternionMultiply(quatZ, XMQuaternionMultiply(quatY, quatX));
    rotationQuat = XMQuaternionNormalize(rotationQuat);  // 정규화 필수

    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    FMatrix result = FMatrix::Identity;  // 기본값 설정 (단위 행렬)

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.M[i][j] = rotationMatrix.r[i].m128_f32[j];  // XMMATRIX에서 FMatrix로 값 복사
        }
    }
    return result;
}

FVector JungleMath::VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // Distance to reach
    const FVector Dist = Target - Current;

    // If distance is too small, just set the desired location
    if (Dist.SizeSquared() < SMALL_NUMBER)
    {
        return Target;
    }

    // Delta Move, Clamp so we do not over shoot.
    const FVector	DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

    return Current + DeltaMove;
}

// 일정 속도 보간 (FMath::VInterpToConstantTo 유사)
FVector JungleMath::VInterpToConstant(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
    const FVector Delta = Target - Current;
    const float DeltaM = Delta.Length();
    const float MaxStep = InterpSpeed * DeltaTime;

    if (DeltaM > MaxStep)
    {
        if (MaxStep > 0.f)
        {
            const FVector DeltaN = Delta / DeltaM;
            return Current + DeltaN * MaxStep;
        }
        else
        {
            return Current;
        }
    }

    return Target;
}

FQuat JungleMath::QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // If the values are nearly equal, just return Target and assume we have reached our destination.
    if (Current.Equals(Target))
    {
        return Target;
    }

    return FQuat::Slerp(Current, Target, FMath::Clamp<float>(InterpSpeed * DeltaTime, 0.f, 1.f));
}

FQuat JungleMath::QInterpConstantTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // If the values are nearly equal, just return Target and assume we have reached our destination.
    if (Current.Equals(Target))
    {
        return Target;
    }

    float DeltaInterpSpeed = FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
    float AngularDistance = FMath::Max<float>(SMALL_NUMBER, (float)(Target.AngularDistance(Current)));
    float Alpha = FMath::Clamp<float>(DeltaInterpSpeed / AngularDistance, 0.f, 1.f);

    return FQuat::Slerp(Current, Target, Alpha);
}

FVector JungleMath::DecomposeTranslation(const FMatrix& M)
{
    // CreateModelMatrix 에서 translation 은 M[3][0..2] 에 저장했습니다.
    return FVector(
        M.M[3][0],
        M.M[3][1],
        M.M[3][2]
    );
}

FQuat JungleMath::DecomposeRotation(const FMatrix& M)
{
    // 먼저 scale 로부터 3x3 회전 요소만 분리
    FVector scale = DecomposeScale(M);

    // 정규화된 회전 행렬 R
    float r00 = M.M[0][0] / scale.X;
    float r01 = M.M[0][1] / scale.Y;
    float r02 = M.M[0][2] / scale.Z;
    float r10 = M.M[1][0] / scale.X;
    float r11 = M.M[1][1] / scale.Y;
    float r12 = M.M[1][2] / scale.Z;
    float r20 = M.M[2][0] / scale.X;
    float r21 = M.M[2][1] / scale.Y;
    float r22 = M.M[2][2] / scale.Z;

    // 회전 매트릭스 → 쿼터니언 변환 (표준 알고리즘)
    float trace = r00 + r11 + r22;
    float qw, qx, qy, qz;

    if (trace > 0.0f)
    {
        float s = std::sqrt(trace + 1.0f) * 2.0f; // s=4*qw
        qw = 0.25f * s;
        qx = (r21 - r12) / s;
        qy = (r02 - r20) / s;
        qz = (r10 - r01) / s;
    }
    else if ((r00 > r11) && (r00 > r22))
    {
        float s = std::sqrt(1.0f + r00 - r11 - r22) * 2.0f; // s=4*qx
        qw = (r21 - r12) / s;
        qx = 0.25f * s;
        qy = (r01 + r10) / s;
        qz = (r02 + r20) / s;
    }
    else if (r11 > r22)
    {
        float s = std::sqrt(1.0f + r11 - r00 - r22) * 2.0f; // s=4*qy
        qw = (r02 - r20) / s;
        qx = (r01 + r10) / s;
        qy = 0.25f * s;
        qz = (r12 + r21) / s;
    }
    else
    {
        float s = std::sqrt(1.0f + r22 - r00 - r11) * 2.0f; // s=4*qz
        qw = (r10 - r01) / s;
        qx = (r02 + r20) / s;
        qy = (r12 + r21) / s;
        qz = 0.25f * s;
    }

    FQuat q(qx, qy, qz, qw);
    q.Normalize();
    return q;
}

FVector JungleMath::DecomposeScale(const FMatrix& M)
{
    // Scale 은 상위 3x3 매트릭스의 각 열 벡터 길이로부터 구합니다.
    float scaleX = std::sqrt(
        M.M[0][0] * M.M[0][0] +
        M.M[1][0] * M.M[1][0] +
        M.M[2][0] * M.M[2][0]
    );
    float scaleY = std::sqrt(
        M.M[0][1] * M.M[0][1] +
        M.M[1][1] * M.M[1][1] +
        M.M[2][1] * M.M[2][1]
    );
    float scaleZ = std::sqrt(
        M.M[0][2] * M.M[0][2] +
        M.M[1][2] * M.M[1][2] +
        M.M[2][2] * M.M[2][2]
    );
    return FVector(scaleX, scaleY, scaleZ);
}
