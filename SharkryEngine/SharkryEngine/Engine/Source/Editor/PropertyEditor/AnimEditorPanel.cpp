#include "AnimEditorPanel.h"
#include "Engine/Animation/AnimSequence.h"
#include "include/ImNeoSequencer/imgui_neo_sequencer.h"


void AnimEditorPanel::Render()
{
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];

    // --- 1) DisplaySize 취득
    const ImVec2 DisplaySize = IO.DisplaySize;

    // --- 2) 패널 크기 계산
    const float PanelWidth = Width * 4.0f;
    const float PanelHeight = Height * 3.0f;

    // --- 3) 왼쪽 하단 위치 계산
    const ImVec2 PanelPos(0.0f, DisplaySize.y - PanelHeight);


    /* 윈도우 위치 고정 */
    ImGui::SetNextWindowPos(PanelPos, ImGuiCond_Always);

    /* 윈도우 크기 고정 */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* 윈도우 플래그 */
    constexpr ImGuiWindowFlags PanelFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBackground;

    /* 렌더 시작 */
    ImGui::Begin("Animation Panel", nullptr, PanelFlags);

    CreateAnimNotifyControl();

    ImGui::End();
}

void AnimEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void AnimEditorPanel::CreateAnimNotifyControl()
{
    int32_t currentFrame = 0;
    int32_t startFrame = -10;
    int32_t endFrame = 64;
    static bool transformOpen = false;
    std::vector<ImGui::FrameIndexType> keys = { 0, 10, 24 };
    bool doDelete = false;

    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
        ImGuiNeoSequencerFlags_EnableSelection |
        ImGuiNeoSequencerFlags_Selection_EnableDragging |
        ImGuiNeoSequencerFlags_Selection_EnableDeletion))
    {
        if (ImGui::BeginNeoGroup("Transform", &transformOpen))
        {

            if (ImGui::BeginNeoTimelineEx("Position"))
            {
                for (auto&& v : keys)
                {
                    ImGui::NeoKeyframe(&v);
                    // Per keyframe code here
                }


                if (doDelete)
                {
                    uint32_t count = ImGui::GetNeoKeyframeSelectionSize();

                    ImGui::FrameIndexType* toRemove = new ImGui::FrameIndexType[count];

                    ImGui::GetNeoKeyframeSelection(toRemove);

                    //Delete keyframes from your structure
                }
                ImGui::EndNeoTimeLine();
            }
            ImGui::EndNeoGroup();
        }

        ImGui::EndNeoSequencer();
    }
}

void AnimEditorPanel::SetAnimSequence(UAnimSequence* InAnimSequence)
{
    AnimSequence = InAnimSequence;
}
