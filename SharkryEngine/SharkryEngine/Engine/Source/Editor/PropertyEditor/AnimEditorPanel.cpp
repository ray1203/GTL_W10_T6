#include "AnimEditorPanel.h"
#include "Engine/Animation/AnimSequence.h"
#include "Engine/Animation/AnimInstances/AnimSingleNodeInstance.h"
#include "Engine/Animation/AnimInstances/MyAnimInstance.h"
#include "Engine/Animation/AnimNotify.h"
#include "Engine/Animation/AnimData/AnimDataModel.h"
#include "include/ImNeoSequencer/imgui_neo_sequencer.h"
#include "World/World.h"
#include "Classes/Actors/ASkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Engine/EditorEngine.h"


void AnimEditorPanel::Render()
{
    if (!CheckAnimationSelected()) 
    {
        return;
    }
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
    UAnimDataModel* AnimDataModel = AnimSequence->GetDataModel();
    
    // Sequence와 AnimInstance에서 정보 가져다가 사용
    TArray<FAnimNotifyEvent>& NotifyEvents = AnimSequence->Notifies;

    int32_t currentFrame = 0;
    int32_t startFrame = 0;
    int32_t endFrame = AnimDataModel->NumberOfFrames;
    float playLength = AnimDataModel->PlayLength;
    static bool transformOpen = true;

    bool doDelete = false;
    TArray<int> RemoveNotifiesIndex;

    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
        ImGuiNeoSequencerFlags_EnableSelection |
        ImGuiNeoSequencerFlags_Selection_EnableDragging |
        ImGuiNeoSequencerFlags_Selection_EnableDeletion))
    {

        if (ImGui::BeginNeoGroup("Notifies", &transformOpen))
        {
            for (int i = 1; i <= AnimSequence->GetNotifyTrackCount(); i++)
            {
                char timelineLabel[64];
                snprintf(
                    timelineLabel,
                    sizeof(timelineLabel),
                    "%d##Position%d",   // 화면엔 %d, ID엔 Position%d
                    i,               // 보여줄 숫자 (1,2,3…)
                    i                    // 내부 ID uniqueness
                );

                if (ImGui::BeginNeoTimelineEx(timelineLabel))
                {
                    for (int j = 0; j < AnimSequence->Notifies.Num(); j++)
                    {
                        FAnimNotifyEvent& Notify = AnimSequence->Notifies[j];
                        if (Notify.TrackNum == i)
                        {
                            if (Notify.NotifyMode == ENotifyMode::Single)
                            {
                                // Notify의 TriggerTime을 frame으로 변환
                                Notify.UpdateTriggerFrame(playLength, AnimDataModel->NumberOfFrames);
                                ImGui::NeoKeyframe(&Notify.TriggerFrame);
                                if (doDelete && ImGui::IsNeoKeyframeSelected()) 
                                {
                                    RemoveNotifiesIndex.Add(j);
                                }
                                Notify.UpdateTriggerTime(AnimDataModel->NumberOfFrames);
                            }
                            else // NotifyState인 경우
                            {
                                bool open = true;
                                Notify.UpdateTriggerFrame(playLength, AnimDataModel->NumberOfFrames);
                                ImGui::BeginNeoNotifyState(*Notify.NotifyName.ToString(),
                                    &Notify.TriggerFrame, &Notify.Duration, &open);
                                ImGui::EndNeoNotifyState();
                            }
                        }
                    }
                    ImGui::EndNeoTimeLine();
                }
            }
            
            ImGui::EndNeoGroup();
        }

        RemoveNotifiesIndex.Sort();
        for (int i = RemoveNotifiesIndex.Num() - 1; i >= 0; i--) 
        {
            AnimSequence->Notifies.RemoveAt(RemoveNotifiesIndex[i]);
        }

        ImGui::EndNeoSequencer();
    }
}

bool AnimEditorPanel::CheckAnimationSelected()
{
    AnimSequence = nullptr;

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    // 선택된 스켈레탈 애니메이션 Instance 가져오기
    /* 선택된 스켈레탈 메시 컴포넌트 가져오기 */
    AActor* SelectedActor = Engine->GetSelectedActor();

    for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
    {
        if (ASkeletalMeshActor* SkeletalActor = Cast<ASkeletalMeshActor>(Actor))
        {
            SelectedActor = SkeletalActor;
        }
    }

    if (SelectedActor == nullptr) 
    {
        return false;
    }

    ASkeletalMeshActor* SkeletalMeshActor = nullptr;
    SkeletalMeshActor = Cast<ASkeletalMeshActor>(SelectedActor);
    if (SkeletalMeshActor == nullptr) 
    {
        return false;
    }

    // 선택된 스켈레탈 애니메이션 Instance와 Sequence 가져오기
    USkeletalMeshComponent* SkeletalComp = SkeletalMeshActor->GetSkeletalMeshComponent();
    if (SkeletalComp == nullptr)
    {
        return false;
    }
    
    UMyAnimInstance* AnimInstance = Cast<UMyAnimInstance>(SkeletalComp->GetAnimInstance());

    if (AnimInstance == nullptr)
    {
        return false;
    }
    
    AnimSequence = AnimInstance->GetAnimationSequence();
    if (AnimSequence == nullptr)
    {
        return false;
    }

    return true;
}

void AnimEditorPanel::SetAnimSequence(UAnimSequence* InAnimSequence)
{
    AnimSequence = InAnimSequence;
}
