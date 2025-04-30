#pragma once
#include <filesystem>

#include "HAL/PlatformType.h"

struct ImVec2;

class CurveManager
{
public:
    static void LoadCurve(std::filesystem::path FilePath, uint32 PointCount, ImVec2* OutCurves);
    static void ResetCurve(ImVec2* Curves, ImVec2 Min, ImVec2 Max, ImVec2 End, uint32 MaxPoint);
    static float CurveValue(float p, int maxpoints, const ImVec2* points);
};
