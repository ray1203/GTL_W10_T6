#include "LastWarUI.h"
#include "ImGUI/imgui.h"
#include "Games/LastWar/Characters/PlayerCharacter.h"
#include "Delegates/DelegateCombination.h"
#include "World/World.h"
#include "Engine/Engine.h"
#include "Games/LastWar/Core/Spawner.h"
#include "Audio/AudioManager.h"

bool LastWarUI::bShowGameOver = false;

LastWarUI::LastWarUI()
{
}

LastWarUI::~LastWarUI()
{
}

void LastWarUI::Initialize()
{

}

void LastWarUI::Render()
{
    // 1) 플레이어 캐릭터 가져오기
    APlayerController* Controller = GEngine->ActiveWorld->GetFirstPlayerController();

    if (Controller)
    {
        APlayerCharacter* Player = Cast<APlayerCharacter>(Controller->GetCharacter());

        if (Player)
        {
            Player->OnDeath.AddDynamic(this, &LastWarUI::GameOver);

            // 2) 현재 체력 문자열 준비
            char buf[64];
            float hp = Player->GetHealth();
            snprintf(buf, sizeof(buf), "HP: %.0f", hp);

            // 3) 화면 정보
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 displaySize = io.DisplaySize;

            // 4) 텍스트 크기 계산
            ImVec2 textSize = ImGui::CalcTextSize(buf);

            // 5) 위치 계산 (상단 센터, Y는 10px 정도 아래)
            ImVec2 pos;
            pos.x = (displaySize.x - textSize.x) * 0.5f;
            pos.y = 10.0f;

            // 6) 투명 배경의 작은 창을 띄워서 그 위치에만 텍스트 렌더링
            ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
            ImGui::Begin("##HealthOverlay", nullptr,
                ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoBackground
                | ImGuiWindowFlags_NoInputs
                | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SetWindowFontScale(2.0f);
            ImGui::TextUnformatted(buf);
            ImGui::SetWindowFontScale(1.0f);
            ImGui::End();
        }
    }

    if (bShowGameOver)
    {
        ImGuiIO& io = ImGui::GetIO();

        // 1) 풀스크린, 배경·테두리·입력 모두 제거
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration    // 제목바, 테두리, 스크롤바 전부 없앰
            | ImGuiWindowFlags_NoBackground    // 배경 없앰
            | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##GameOverOverlay", nullptr, flags);

        // 2) 텍스트 크기 구해서 화면 가운데 좌표 계산
        const char* text = "GAME OVER";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 pos;
        pos.x = (io.DisplaySize.x - textSize.x * 4) * 0.5f;
        pos.y = (io.DisplaySize.y - textSize.y * 4) * 0.5f;

        // 3) 커서 위치 세팅 후 텍스트 출력
        ImGui::SetCursorScreenPos(pos);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 200)); // 반투명 빨간
        ImGui::SetWindowFontScale(4.0f);
        ImGui::TextUnformatted(text);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        const char* btnLabel = "Restart";
        ImVec2 btnSize = ImVec2(120, 50);
        ImVec2 btnPos;
        btnPos.x = (io.DisplaySize.x - btnSize.x) * 0.5f;
        btnPos.y = pos.y + textSize.y + 100.0f;  // 텍스트 아래 100px
        ImGui::SetCursorScreenPos(btnPos);

        if (ImGui::Button(btnLabel, btnSize))
        {
            // 버튼 클릭 시 호출되는 함수
            RestartGame();
        }

        ImGui::End();
    }
}

void LastWarUI::Release()
{
}

void LastWarUI::StartGame()
{
}

void LastWarUI::GameOver()
{
    bShowGameOver = true;
    AudioManager::Get().StopBgm();
}

void LastWarUI::RestartGame()
{
    bShowGameOver = false;
    AudioManager::Get().PlayBgm(EAudioType::MainTheme);
}
