#include "CurveManager.h"

#include <fstream>

#include "HAL/PlatformType.h"
#include "ImGUI/imgui.h"

void CurveManager::LoadCurve(std::filesystem::path FilePath, uint32 PointCount, ImVec2* OutCurves)
{
    try
    {
        std::ifstream File(FilePath);
        if (File.is_open())
        {
            // file 파싱
                    
            // file << "Time,Value,\n";
            // file << key.x << "," << key.y << ",\n";
            std::string line;
            std::getline(File, line); // "Time,Value,"
            uint32 index = 0;
            while (std::getline(File, line)) {
                if (index >= PointCount) {
                    //std::cout << "Reached max array size. Breaking.\n";
                    break;
                }

                std::stringstream ss(line);
                std::string timeStr, valueStr;

                if (!std::getline(ss, timeStr, ',') || !std::getline(ss, valueStr, ',')) {
                    //std::cout << "Line ended prematurely. Breaking.\n";
                    break;
                }

                try {
                    float time = std::stof(timeStr);
                    float value = std::stof(valueStr);
                    OutCurves[index] = ImVec2(time, value);
                    index++;
                } catch (...) {
                    //std::cerr << "Invalid float format in line: " << line << std::endl;
                    break;
                }
            } 
            
            File.close();
        }
        else
        {
            // TODO: Error Check

            MessageBoxA(nullptr, "Failed to Load Curve File for writing: ", "Error", MB_OK | MB_ICONERROR);
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        // TODO: Error Check
        MessageBoxA(nullptr, "Failed to Load Curve File for writing: ", "Error", MB_OK | MB_ICONERROR);
    }
}

void CurveManager::ResetCurve(ImVec2* Curves, ImVec2 Min, ImVec2 Max, ImVec2 End, uint32 MaxPoint)
{
    for (uint32 i = 0; i < MaxPoint; i++)
    {
        Curves[i] = ImVec2(0, 0);
    }
                
    if (MaxPoint > 1)
    {
        Curves[0] = Min;
    }

    if (MaxPoint > 2)
    {
        Curves[1] = Max;
    }

    if (MaxPoint > 3)
    {
        Curves[2] = End;
    }
}

float CurveManager::CurveValue(float p, int maxpoints, const ImVec2* points)
{
    if (maxpoints < 2 || points == 0)
        return 0;
    if (p < 0)
        return points[0].y;

    int left = 0;
    while (left < maxpoints && points[left].x < p && points[left].x != -1)
        left++;
    if (left)
        left--;

    if (left == maxpoints - 1)
        return points[maxpoints - 1].y;

    float d = (p - points[left].x) / (points[left + 1].x - points[left].x);

    return points[left].y + (points[left + 1].y - points[left].y) * d;
}
