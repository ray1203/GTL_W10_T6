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

    // 유틸리티 함수
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
        FMatrix OutMatrix; // 초기화되지 않은 상태일 수 있으므로 모든 요소 설정 필요

        FVector XAxisFBX(InFbxMatrix.Get(0, 0), InFbxMatrix.Get(0, 1), InFbxMatrix.Get(0, 2));
        FVector YAxisFBX(InFbxMatrix.Get(1, 0), InFbxMatrix.Get(1, 1), InFbxMatrix.Get(1, 2));
        FVector ZAxisFBX(InFbxMatrix.Get(2, 0), InFbxMatrix.Get(2, 1), InFbxMatrix.Get(2, 2));
        FVector TranslationFBX(InFbxMatrix.Get(3, 0), InFbxMatrix.Get(3, 1), InFbxMatrix.Get(3, 2));

        FVector XAxisUE(XAxisFBX.X, XAxisFBX.Z, XAxisFBX.Y);
        FVector YAxisUE(ZAxisFBX.X, ZAxisFBX.Z, ZAxisFBX.Y); // FBX Z maps to UE Y
        FVector ZAxisUE(YAxisFBX.X, YAxisFBX.Z, YAxisFBX.Y); // FBX Y maps to UE Z
        FVector TranslationUE(TranslationFBX.X, TranslationFBX.Z, TranslationFBX.Y);

        OutMatrix.M[0][0] = XAxisUE.X;
        OutMatrix.M[1][0] = XAxisUE.Y;
        OutMatrix.M[2][0] = XAxisUE.Z;
        OutMatrix.M[3][0] = 0.0f;

        OutMatrix.M[0][1] = YAxisUE.X;
        OutMatrix.M[1][1] = YAxisUE.Y;
        OutMatrix.M[2][1] = YAxisUE.Z;
        OutMatrix.M[3][1] = 0.0f;

        OutMatrix.M[0][2] = ZAxisUE.X;
        OutMatrix.M[1][2] = ZAxisUE.Y;
        OutMatrix.M[2][2] = ZAxisUE.Z;
        OutMatrix.M[3][2] = 0.0f;

        OutMatrix.M[0][3] = TranslationUE.X;
        OutMatrix.M[1][3] = TranslationUE.Y;
        OutMatrix.M[2][3] = TranslationUE.Z;
        OutMatrix.M[3][3] = 1.0f;

        return OutMatrix;
    }
};

inline FArchive& operator<<(FArchive& Ar, FMatrix& M)
{
    Ar << M.M[0][0] << M.M[0][1] << M.M[0][2] << M.M[0][3];
    Ar << M.M[1][0] << M.M[1][1] << M.M[1][2] << M.M[1][3];
    Ar << M.M[2][0] << M.M[2][1] << M.M[2][2] << M.M[2][3];
    Ar << M.M[3][0] << M.M[3][1] << M.M[3][2] << M.M[3][3];
    return Ar;
}

