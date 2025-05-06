#include "ViewerControlEditorPanel.h"

#include "World/World.h"

#include "Actors/Player.h"
#include "Actors/FireballActor.h"

#include "Components/Light/LightComponent.h"
#include "Components/SphereComp.h"
#include "Components/ParticleSubUVComponent.h"
#include "Components/TextComponent.h"

#include "Engine/FLoaderOBJ.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "tinyfiledialogs/tinyfiledialogs.h"

#include "Actors/Cube.h"

#include "Engine/EditorEngine.h"
#include <Actors/HeightFogActor.h>

#include "ShowFlag.h"
#include "Actors/PointLightActor.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/SpotLightActor.h"
#include "Actors/AmbientLightActor.h"
#include "Actors/ASkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Games/LastWar/Core/Spawner.h"
#include "ImGUI/imgui.h"
#include "UObject/Casts.h"
#include "FLoaderFBX.h"

void ViewerControlEditorPanel::Render()
{
    /* Pre Setup */
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);

    const float PanelWidth = (Width) * 1.5f;
    constexpr float PanelHeight = 45.0f;

    constexpr float PanelPosX = 1.0f;
    constexpr float PanelPosY = 1.0f;

    constexpr ImVec2 MinSize(300, 50);
    constexpr ImVec2 MaxSize(FLT_MAX, 50);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    /* Render Start */
    ImGui::Begin("Control Panel", nullptr, PanelFlags);

    CreateMenuButton(IconSize, IconFont);
    ImGui::SameLine();
    CreateFlagButton();
    ImGui::SameLine();
    CreateLightSpawnButton(IconSize, IconFont);
    ImGui::SameLine();

    ImGui::End();
}

void ViewerControlEditorPanel::CreateMenuButton(const ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();

    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 80), ImGuiCond_Always);

        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const* lFilterPatterns[1] = { "*.obj" };
                const char* FileName = tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns, "Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << '\n';

                    if (FManagerOBJ::CreateStaticMesh(FileName) == nullptr)
                    {
                        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                    }
                }
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("프로그램 종료");
        }

        const ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(Center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("프로그램 종료", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("정말 프로그램을 종료하시겠습니까?");

            ImGui::Separator();

            const float ContentWidth = ImGui::GetWindowContentRegionMax().x;
            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));
            if (ImGui::Button("OK", ImVec2(80, 0)))
            {
                PostQuitMessage(0);
            }
            ImGui::SameLine();
            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor::HSV(0.0f, 1.0f, 1.0f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor::HSV(0.0f, 0.9f, 1.0f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor::HSV(0.0f, 1.0f, 1.0f)));
            if (ImGui::Button("Cancel", ImVec2(80, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }

        ImGui::End();
    }
}

void ViewerControlEditorPanel::CreateFlagButton()
{
    const std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    ImGui::SameLine();

    const char* ViewModeNames[] = {
        "Lit_Gouraud", "Lit_Lambert", "Lit_Phong",
        "Unlit", "Wireframe",
        "World Normal"
    };

    constexpr uint32 ViewModeCount = std::size(ViewModeNames);
    const int RawViewMode = static_cast<int>(ActiveViewport->GetViewMode());
    const int SafeIndex = (RawViewMode >= 0) ? (RawViewMode % ViewModeCount) : 0;
    FString ViewModeControl = ViewModeNames[SafeIndex];
    const ImVec2 ViewModeTextSize = ImGui::CalcTextSize(GetData(ViewModeControl));

    if (ImGui::Button(GetData(ViewModeControl), ImVec2(30 + ViewModeTextSize.x, 32)))
    {
        ImGui::OpenPopup("ViewModeControl");
    }

    if (ImGui::BeginPopup("ViewModeControl"))
    {
        for (int i = 1; i < IM_ARRAYSIZE(ViewModeNames); i++)
        {
            bool bIsSelected = (static_cast<int>(ActiveViewport->GetViewMode()) == i);
            if (ImGui::Selectable(ViewModeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewMode(static_cast<EViewModeIndex>(i));
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();


    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowFlags");
    }

    const char* Items[] = {"Primitives", "Shadow", "Bone", "SkeletalMesh", "Debug"};
    const uint64 CurFlag = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowFlags"))
    {
        bool Selected[IM_ARRAYSIZE(Items)] =
        {
            static_cast<bool>(CurFlag & EEngineShowFlags::SF_Primitives),
            static_cast<bool>(CurFlag & EEngineShowFlags::SF_Shadow),
            static_cast<bool>(CurFlag & EEngineShowFlags::SF_Bone),
            static_cast<bool>(CurFlag & EEngineShowFlags::SF_SkeletalMesh),
            static_cast<bool>(CurFlag & EEngineShowFlags::SF_AABB),
        };

        for (int i = 0; i < IM_ARRAYSIZE(Items); i++)
        {
            ImGui::Checkbox(Items[i], &Selected[i]);
        }

        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(Selected));
        ImGui::EndPopup();
    }
}

void ViewerControlEditorPanel::OnResize(const HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void ViewerControlEditorPanel::CreateLightSpawnButton(const ImVec2 InButtonSize, ImFont* IconFont)
{
    UWorld* World = GEngine->ActiveWorld;
    const ImVec2 WindowSize = ImGui::GetIO().DisplaySize;

    const float CenterX = (WindowSize.x - InButtonSize.x) / 2.5f;

    ImGui::SetCursorScreenPos(ImVec2(CenterX + 40.0f, 10.0f));
    const char* Text = "Light";
    const ImVec2 TextSize = ImGui::CalcTextSize(Text);
    const ImVec2 Padding = ImGui::GetStyle().FramePadding;
    ImVec2 ButtonSize = ImVec2(
        TextSize.x + Padding.x * 2.0f,
        TextSize.y + Padding.y * 2.0f
    );
    ButtonSize.y = InButtonSize.y;
    if (ImGui::Button("Light", ButtonSize))
    {
        ImGui::OpenPopup("LightGeneratorControl");
    }

    if (ImGui::BeginPopup("LightGeneratorControl"))
    {
        struct LightGeneratorMode
        {
            const char* Label;
            int Mode;
        };

        static constexpr LightGeneratorMode modes[] =
        {
            {.Label = "Generate", .Mode = ELightGridGenerator::Generate },
            {.Label = "Delete", .Mode = ELightGridGenerator::Delete },
            {.Label = "Reset", .Mode = ELightGridGenerator::Reset },
        };

        for (const auto& [Label, Mode] : modes)
        {
            if (ImGui::Selectable(Label))
            {
                switch (Mode)
                {
                case ELightGridGenerator::Generate:
                    LightGridGenerator.GenerateLight(World);
                    break;
                case ELightGridGenerator::Delete:
                    LightGridGenerator.DeleteLight(World);
                    break;
                case ELightGridGenerator::Reset:
                    LightGridGenerator.Reset(World);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }
}

uint64 ViewerControlEditorPanel::ConvertSelectionToFlags(const bool Selected[])
{
    uint64 Flags = EEngineShowFlags::None;

    if (Selected[0])
    {
        Flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    }
    if (Selected[1])
    {
        Flags |= static_cast<uint64>(EEngineShowFlags::SF_Shadow);
    }
    if (Selected[2])
    {
        Flags |= static_cast<uint64>(EEngineShowFlags::SF_Bone);
    }
    if (Selected[3])
    {
        Flags |= static_cast<uint64>(EEngineShowFlags::SF_SkeletalMesh);
    }
    if (Selected[4])
    {
        Flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    }

    return Flags;
}
