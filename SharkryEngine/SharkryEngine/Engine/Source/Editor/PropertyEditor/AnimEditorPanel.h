#pragma once
#include "UnrealEd/EditorPanel.h"

class UAnimSequence;

class AnimEditorPanel : public UEditorPanel 
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void CreateAnimNotifyControl();

public:
    void SetAnimSequence(UAnimSequence* InAnimSequence);

private:
    float Width = 300, Height = 100;
    bool bOpenMenu = false;
    bool bShowImGuiDemoWindow = false; // 데모 창 표시 여부를 관리하는 변수
    
    UAnimSequence* AnimSequence = nullptr;
};
