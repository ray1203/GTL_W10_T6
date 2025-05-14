#include "FBXConversionUtil.h"

namespace FBX
{
    FVector ConvertFbxPosition(const FbxVector4& Vector)
    {
        return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
    }

    FVector ConvertFbxNormal(const FbxVector4& Vector)
    {
        return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
    }

    FVector2D ConvertFbxUV(const FbxVector2& Vector)
    {
        return FVector2D(static_cast<float>(Vector[0]), 1.0f - static_cast<float>(Vector[1]));
    }

    FLinearColor ConvertFbxColorToLinear(const FbxDouble3& Color)
    {
        return FLinearColor(static_cast<float>(Color[0]), static_cast<float>(Color[1]), static_cast<float>(Color[2]), 1.0f);
    }

    FMatrix ConvertFbxAMatrixToFMatrix(const FbxAMatrix& FbxMatrix) {
        FMatrix Result;
        // 간단한 유효성 검사 (모든 요소가 유한한지)
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                double val = FbxMatrix.Get(i, j);
                if (!FMath::IsFinite(val))
                {
                    return FMatrix::Identity; // 문제가 있으면 단위 행렬 반환
                }
                Result.M[i][j] = static_cast<float>(val);
            }
        }
        return Result;
    }

    FbxAMatrix ConvertFMatrixToFbxAMatrix(const FMatrix& InMatrix)
    {
        FbxAMatrix OutMatrix;

        for (int Row = 0; Row < 4; ++Row)
        {
            for (int Col = 0; Col < 4; ++Col)
            {
                OutMatrix.mData[Row][Col] = static_cast<double>(InMatrix.M[Row][Col]);
            }
        }

        return OutMatrix;
    }
    FString FbxTransformToString(const FbxAMatrix& Matrix)
    {
        FbxVector4 T = Matrix.GetT(); // Translation
        FbxVector4 R = Matrix.GetR(); // Rotation (Euler)
        FbxVector4 S = Matrix.GetS(); // Scaling

        return FString::Printf(TEXT("T(%.2f, %.2f, %.2f) | R(%.2f, %.2f, %.2f) | S(%.2f, %.2f, %.2f)"),
            T[0], T[1], T[2],
            R[0], R[1], R[2],
            S[0], S[1], S[2]);
    }
}
