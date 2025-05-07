#include "Console.h"
#include <cstdarg>
#include <cstdio>
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/Engine.h"
#include "Renderer/UpdateLightBufferPass.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/Light/AmbientLightComponent.h"
#include "ImGUI/imgui.h"
#include "Stats/ProfilerStatsManager.h"
#include "Stats/GPUTimingManager.h"

void StatOverlay::ToggleStat(const std::string& Command)
{
    if (Command == "stat fps")
    {
        ShowFPS = true;
        ShowRender = true;
    }
    else if (Command == "stat memory")
    {
        ShowMemory = true;
        ShowRender = true;
    }
    else if (Command == "stat light")
    {
        ShowLight = true;
        ShowRender = true;
    }
    else if (Command == "stat none")
    {
        ShowFPS = false;
        ShowMemory = false;
        ShowLight = false;
        ShowRender = false;
    }
}

void StatOverlay::Render(ID3D11DeviceContext* Context, UINT Width, UINT Height) const
{
    if (!ShowRender)
    {
        return;
    }

    const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
    // 창 크기를 화면의 50%로 설정합니다.
    const ImVec2 WindowSize(DisplaySize.x * 0.5f, DisplaySize.y * 0.5f);
    // 창을 중앙에 배치하기 위해 위치를 계산합니다.
    const ImVec2 WindowPos((DisplaySize.x - WindowSize.x) * 0.5f, (DisplaySize.y - WindowSize.y) * 0.5f);

    ImGui::SetNextWindowPos(WindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Begin("Stat Overlay", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);

    if (ShowFPS)
    {
        static float LastTime = ImGui::GetTime();
        static float FPS = 0.0f;
        static float FrameTimeMS = 0.0f;

        float CurrentTime = ImGui::GetTime();
        float DeltaTime = CurrentTime - LastTime;

        FPS = 1.0f / DeltaTime;
        FrameTimeMS = DeltaTime * 1000.0f;

        ImGui::Text("%.2f FPS", FPS);
        ImGui::Text("%.2f ms", FrameTimeMS);
        ImGui::Text("\n");

        LastTime = CurrentTime;
    }

    if (ShowMemory)
    {
        ImGui::Text("Allocated Object Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Object>());
        ImGui::Text("Allocated Object Memory: %llu B", FPlatformMemory::GetAllocationBytes<EAT_Object>());
        ImGui::Text("Allocated Container Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Container>());
        ImGui::Text("Allocated Container memory: %llu B", FPlatformMemory::GetAllocationBytes<EAT_Container>());
    }

    if (ShowLight)
    {
        // @todo Find Better Way to Get Light Count
        int NumPointLights = 0;
        int NumSpotLights = 0;
        for (const auto iter : TObjectRange<ULightComponentBase>())
        {
            if (iter->GetWorld() == GEngine->ActiveWorld)
            {
                if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
                {
                    NumPointLights++;
                }
                else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
                {
                    NumSpotLights++;
                }
                /*
                else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
                {
                    DirectionalLights.Add(DirectionalLight);
                }
                else if (UAmbientLightComponent* AmbientLight = Cast<UAmbientLightComponent>(iter))
                {
                    AmbientLights.Add(AmbientLight);
                }
                */
            }
        }
        ImGui::Text("[ Light Counters ]\n");
        ImGui::Text("Point Light: %d", NumPointLights);
        ImGui::Text("Spot Light: %d", NumSpotLights);
        ImGui::Text("\n");
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

void FEngineProfiler::SetGPUTimingManager(FGPUTimingManager* InGPUTimingManager)
{
    GPUTimingManager = InGPUTimingManager;
}

void FEngineProfiler::Render(ID3D11DeviceContext* Context, UINT Width, UINT Height)
{
    if (!bShowWindow) return;

    // Example positioning: Top-left corner
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Engine Profiler", &bShowWindow))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("ProfilerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("GPU (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (const auto& [DisplayName, CPUStatName, GPUStatName] : TrackedScopes)
        {
            const double CPUTimeMs = FProfilerStatsManager::GetCpuStatMs(CPUStatName);
            double GPUTimeMs = GPUTimingManager->GetElapsedTimeMs(TStatId(GPUStatName));

            FString CPUText = (CPUTimeMs >= 0.0) ? FString::Printf(TEXT("%.3f"), CPUTimeMs) : TEXT("---");
            FString GPUText;

            if (GPUTimeMs == -1.0) GPUText = TEXT("Disjoint");
            else if (GPUTimeMs == -2.0) GPUText = TEXT("Waiting");
            else if (GPUTimeMs < 0.0) GPUText = TEXT("---");
            else GPUText = FString::Printf(TEXT("%.3f"), GPUTimeMs);

            ImGui::TableNextRow();

            // Scope 열
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", *DisplayName);

            // CPU (ms) 열 - 우측 정렬
            ImGui::TableSetColumnIndex(1);
            float CPUTextWidth = ImGui::CalcTextSize(*CPUText).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - CPUTextWidth);
            ImGui::TextUnformatted(*CPUText);

            // GPU (ms) 열 - 우측 정렬
            ImGui::TableSetColumnIndex(2);
            float GPUTextWidth = ImGui::CalcTextSize(*GPUText).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - GPUTextWidth);
            ImGui::TextUnformatted(*GPUText);
        }

        ImGui::EndTable();
    }



    ImGui::End();
}

void FEngineProfiler::RegisterStatScope(const FString& DisplayName, const FName& CPUStatName, const FName& GPUStatName)
{
    TrackedScopes.Add({ DisplayName, CPUStatName, GPUStatName });
}

// 싱글톤 인스턴스 반환
Console& Console::GetInstance() {
    static Console Instance;
    return Instance;
}

// 로그 초기화
void Console::Clear() {
    Items.Empty();
}

// 로그 추가
void Console::AddLog(const LogLevel Level, const char* Format, ...) {
    char Buf[1024];
    va_list args;
    va_start(args, Format);
    vsnprintf(Buf, sizeof(Buf), Format, args);
    va_end(args);

    Items.Add({ Level, std::string(Buf) });
    ScrollToBottom = true;
}

// 콘솔 창 렌더링
void Console::Draw() {
    if (!bWasOpen)
    {
        return;
    }

    static ImGuiTextFilter Filter; // 필터링을 위한 ImGuiTextFilter
    
    // 창 크기 및 위치 계산
    const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;

    // 콘솔 창의 크기와 위치 설정
    const float ExpandedHeight = DisplaySize.y * 0.4f; // 확장된 상태일 때 높이 (예: 화면의 40%)
    constexpr float CollapsedHeight = 30.0f;               // 축소된 상태일 때 높이
    const float CurrentHeight = bExpand ? ExpandedHeight : CollapsedHeight;

    // 왼쪽 하단에 위치하도록 계산 (창의 좌측 하단이 화면의 좌측 하단에 위치)
    const ImVec2 WindowSize(DisplaySize.x * 0.5f, CurrentHeight); // 폭은 화면의 40%
    const ImVec2 WindowPos(0, DisplaySize.y - CurrentHeight);

    // 창 위치와 크기를 고정
    ImGui::SetNextWindowPos(WindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Always);

    // 창을 표시하고 닫힘 여부 확인
    Overlay.Render(FEngineLoop::GraphicDevice.DeviceContext, Width, Height);

    bExpand = ImGui::Begin("Console", &bWasOpen);
    if (!bExpand)
    {
        // 창을 접었을 경우 UI를 표시하지 않음
        ImGui::End();
        return;
    }

    // 버튼 UI (로그 수준별 추가)
    if (ImGui::Button("Clear"))
    {
        Clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy"))
    {
        ImGui::LogToClipboard();
    }

    ImGui::Separator();

    // 필터 입력 창
    ImGui::Text("Filter:");
    ImGui::SameLine();
    Filter.Draw("##Filter", 100);
    ImGui::SameLine();
    // 로그 수준을 선택할 체크박스
    ImGui::Checkbox("Show Display", &ShowLogTemp);
    ImGui::SameLine();
    ImGui::Checkbox("Show Warning", &ShowWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Show Error", &ShowError);

    ImGui::Separator();

    // 로그 출력 (필터 적용)
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& [Level, Message] : Items)
    {
        if (!Filter.PassFilter(*Message))
        {
            continue;
        }

        // 로그 수준에 맞는 필터링
        if ((Level == LogLevel::Display && !ShowLogTemp) ||
            (Level == LogLevel::Warning && !ShowWarning) ||
            (Level == LogLevel::Error && !ShowError))
        {
            continue;
        }

        // 색상 지정
        ImVec4 Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (Level)
        {
        case LogLevel::Display:
            Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 기본 흰색
            break;
        case LogLevel::Warning:
            Color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // 노란색
            break;
        case LogLevel::Error:
            Color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // 빨간색
            break;
        }

        ImGui::TextColored(Color, "%s", *Message);
    }

    if (ScrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
    }
    ImGui::EndChild();

    ImGui::Separator();

    // 입력창
    bool ReclaimFocus = false;
    if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (InputBuf[0])
        {
            AddLog(LogLevel::Display, ">> %s", InputBuf);
            const std::string Command(InputBuf);
            ExecuteCommand(Command);
            History.Add(std::string(InputBuf));
            HistoryPos = -1;
            ScrollToBottom = true; // 자동 스크롤
        }
        InputBuf[0] = '\0';
        ReclaimFocus = true;
    }

    // 입력 필드에 자동 포커스
    if (ReclaimFocus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

void Console::ExecuteCommand(const std::string& Command)
{
    AddLog(LogLevel::Display, "Executing command: %s", Command.c_str());

    if (Command == "clear")
    {
        Clear();
    }
    else if (Command == "help")
    {
        AddLog(LogLevel::Display, "Available commands:");
        AddLog(LogLevel::Display, " - clear: Clears the console");
        AddLog(LogLevel::Display, " - help: Shows available commands");
        AddLog(LogLevel::Display, " - stat fps: Toggle FPS display");
        AddLog(LogLevel::Display, " - stat memory: Toggle Memory display");
        AddLog(LogLevel::Display, " - stat none: Hide all stat overlays");
    }
    else if (Command.starts_with("stat "))
    {
        Overlay.ToggleStat(Command);
    }
    else
    {
        AddLog(LogLevel::Error, "Unknown command: %s", Command.c_str());
    }
}

void Console::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

