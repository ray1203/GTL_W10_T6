#pragma once

#include "Define.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Color.h"
#include <fbxsdk.h>

namespace FBX
{
    FVector ConvertFbxPosition(const FbxVector4& Vector);
    FVector ConvertFbxNormal(const FbxVector4& Vector);
    FVector2D ConvertFbxUV(const FbxVector2& Vector);
    FLinearColor ConvertFbxColorToLinear(const FbxDouble3& Color);
    FMatrix ConvertFbxAMatrixToFMatrix(const FbxAMatrix& FbxMatrix);
    FbxAMatrix ConvertFMatrixToFbxAMatrix(const FMatrix& InMatrix);

    FString FbxTransformToString(const FbxAMatrix& Matrix);
}
