#include "LastWarUI.h"
#include "Games/LastWar/Characters/PlayerCharacter.h"
#include "Delegates/DelegateCombination.h"
#include "World/World.h"
#include "Engine/Engine.h"
#include "Games/LastWar/Core/Spawner.h"
#include "Audio/AudioManager.h"
#include "ImGUI/imgui.h"

#include "GameFramework/PlayerController.h"

bool LastWarUI::bShowGameOver = false;
int LastWarUI::Score = 0;

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

            // 1) Prepare strings
            char hpBuf[64];
            float Health;
            if (Player->GetHealth() < 0.0f)
            {
                Health = 0.0f;
            }
            else
            {
                Health = Player->GetHealth();
            }
            snprintf(hpBuf, sizeof(hpBuf), "HP: %.0f", Health);

            char scoreBuf[64];
            snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", Score);

            // 2) Calc sizes
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 hpSize = ImGui::CalcTextSize(hpBuf);
            ImVec2 scoreSize = ImGui::CalcTextSize(scoreBuf);

            // 3) Determine window width = max(hpSize.x, scoreSize.x)
            float winW = hpSize.x > scoreSize.x ? hpSize.x : scoreSize.x;

            // 4) Position at top center, y = 10px
            ImVec2 pos;
            pos.x = (io.DisplaySize.x - winW) * 0.5f;
            pos.y = 10.0f;
            ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

            // 5) Begin transparent auto-resize window
            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_NoBackground
                | ImGuiWindowFlags_NoInputs
                | ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::Begin("##StatsOverlay", nullptr, flags);

            // 6) Optional: scale up font
            ImGui::SetWindowFontScale(2.0f);

            // 7) Draw two lines
            ImGui::TextUnformatted(hpBuf);
            ImGui::TextUnformatted(scoreBuf);

            // 8) Restore scale and end
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
    LastWarUI::Score = 0;
}
