#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "LightGridGenerator.h"

struct ImFont;
struct ImVec2;

class ViewerControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    static void CreateFlagButton();
    void CreateLightSpawnButton(ImVec2 InButtonSize, ImFont* IconFont);

private:
    static uint64 ConvertSelectionToFlags(const bool Selected[]);

private:
    float Width = 300, Height = 100;
    bool bOpenMenu = false;
    bool bShowImGuiDemoWindow = false; // 데모 창 표시 여부를 관리하는 변수

    FLightGridGenerator LightGridGenerator;
};

