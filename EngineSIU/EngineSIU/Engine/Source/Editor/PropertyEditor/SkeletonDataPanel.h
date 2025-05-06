#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class USkeleton;

class SkeletonDataPanel : public UEditorPanel
{
public:
    SkeletonDataPanel() = default;

public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void DrawBoneTransformPanel() const;
    USkeleton* Skeleton = nullptr;
private:
    float Width = 0, Height = 0;
};
