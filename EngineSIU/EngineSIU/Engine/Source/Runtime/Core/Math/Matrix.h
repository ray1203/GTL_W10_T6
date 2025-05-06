#pragma once
#include "Serialization/Archive.h"

struct FVector;
struct FVector4;
struct FRotator;
struct FQuat;

#include "Vector.h"
#include "Vector4.h"
#include "fbxsdk.h"
// 4x4 행렬 연산
struct alignas(16) FMatrix
{
public:
    alignas(16) float M[4][4];

public:
    static const FMatrix Identity;

public:
    // 기본 연산자 오버로딩
    FMatrix operator+(const FMatrix& Other) const;
    FMatrix operator-(const FMatrix& Other) const;
    FMatrix operator*(const FMatrix& Other) const;
    FMatrix operator*(float Scalar) const;
    FMatrix operator/(float Scalar) const;
    float* operator[](int row);
    const float* operator[](int row) const;
    bool Equals(const FMatrix& Other, float Tolerance = KINDA_SMALL_NUMBER) const;
    bool operator==(const FMatrix& Other) const;
    // 유틸리티 함수
    void SetOrigin(const FVector& NewOrigin);
    void RemoveTranslation();
    static FMatrix Transpose(const FMatrix& Mat);
    static FMatrix Inverse(const FMatrix& Mat);
    static FMatrix CreateRotationMatrix(float roll, float pitch, float yaw);
    static FMatrix CreateScaleMatrix(float scaleX, float scaleY, float scaleZ);
    static FVector TransformVector(const FVector& v, const FMatrix& m);
    static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
    static FMatrix CreateTranslationMatrix(const FVector& position);

    FVector4 TransformFVector4(const FVector4& vector) const;
    FVector TransformPosition(const FVector& vector) const;

    static FMatrix GetScaleMatrix(const FVector& InScale);
    static FMatrix GetTranslationMatrix(const FVector& InPosition);
    static FMatrix GetRotationMatrix(const FRotator& InRotation);
    static FMatrix GetRotationMatrix(const FQuat& InRotation);

    FQuat ToQuat() const;

    FVector GetScaleVector(float Tolerance = SMALL_NUMBER) const;

    FVector GetTranslationVector() const;

    FMatrix GetMatrixWithoutScale(float Tolerance = SMALL_NUMBER) const;

    void RemoveScaling(float Tolerance = SMALL_NUMBER);

    static FMatrix ConvertFbxAMatrixToFMatrix(const FbxAMatrix& InFbxMatrix)
    {

        FMatrix OutMatrix;

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                OutMatrix.M[i][j] = static_cast<float>(InFbxMatrix.Get(i, j));
            }
        }

        return OutMatrix;
    }
    static inline float Determinant3x3(float a, float b, float c,
        float d, float e, float f,
        float g, float h, float i)
    {
        return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    }
    float Determinant() const
    {
        // 여인수 전개 (첫 번째 행 기준)
        float Det3x3_00 = Determinant3x3(M[1][1], M[1][2], M[1][3], M[2][1], M[2][2], M[2][3], M[3][1], M[3][2], M[3][3]);
        float Det3x3_01 = Determinant3x3(M[1][0], M[1][2], M[1][3], M[2][0], M[2][2], M[2][3], M[3][0], M[3][2], M[3][3]);
        float Det3x3_02 = Determinant3x3(M[1][0], M[1][1], M[1][3], M[2][0], M[2][1], M[2][3], M[3][0], M[3][1], M[3][3]);
        float Det3x3_03 = Determinant3x3(M[1][0], M[1][1], M[1][2], M[2][0], M[2][1], M[2][2], M[3][0], M[3][1], M[3][2]);

        return M[0][0] * Det3x3_00 - M[0][1] * Det3x3_01 + M[0][2] * Det3x3_02 - M[0][3] * Det3x3_03;
    }
    FVector GetAxisX() const { return FVector(M[0][0], M[1][0], M[2][0]); }
    FVector GetAxisY() const { return FVector(M[0][1], M[1][1], M[2][1]); }
    FVector GetAxisZ() const { return FVector(M[0][2], M[1][2], M[2][2]); }

};

inline FArchive& operator<<(FArchive& Ar, FMatrix& M)
{
    Ar << M.M[0][0] << M.M[0][1] << M.M[0][2] << M.M[0][3];
    Ar << M.M[1][0] << M.M[1][1] << M.M[1][2] << M.M[1][3];
    Ar << M.M[2][0] << M.M[2][1] << M.M[2][2] << M.M[2][3];
    Ar << M.M[3][0] << M.M[3][1] << M.M[3][2] << M.M[3][3];
    return Ar;
}

static FMatrix LookAtMatrixAutoUp(const FVector& Eye, const FVector& Target)
{
    FVector Forward = (Target - Eye).GetSafeNormal();
    FVector DefaultUp = FVector::UpVector;

    // 방향과 Up이 너무 일치하면 다른 축 사용
    if (FMath::Abs(Forward.Dot(DefaultUp)) > 0.99f)
    {
        DefaultUp = FVector::RightVector;
    }

    FVector Right = DefaultUp.Cross(Forward).GetSafeNormal();
    FVector Up = Forward.Cross(Right);

    float DX = -Right.Dot(Eye);
    float DY = -Up.Dot(Eye);
    float DZ = -Forward.Dot(Eye);

    return FMatrix{ {
        {Right.X, Up.X, Forward.X, 0.0f},
        {Right.Y, Up.Y, Forward.Y, 0.0f},
        {Right.Z, Up.Z, Forward.Z, 0.0f},
        {DX,      DY,   DZ,        1.0f}
    } };
}
