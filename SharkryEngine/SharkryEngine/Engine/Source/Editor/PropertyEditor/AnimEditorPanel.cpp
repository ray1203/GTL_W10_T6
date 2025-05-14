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
#include "AssetImporter/FBX/FBXManager.h"
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
    // --- Retrieve SkeletalMeshComponent and custom AnimInstance ---
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    USkeletalMeshComponent* SkeletalComp = nullptr;
    for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors) {
        if (ASkeletalMeshActor* SKActor = Cast<ASkeletalMeshActor>(Actor)) {
            SkeletalComp = SKActor->GetComponentByClass<USkeletalMeshComponent>();
            break;
        }
    }
    if (!SkeletalComp) return;
    UMyAnimInstance* MyInst = Cast<UMyAnimInstance>(SkeletalComp->GetAnimInstance());
    if (!MyInst) {
        ImGui::Text("Custom AnimInstance not found");
        return;
    }

    UAnimSequence* AnimSequence = MyInst->GetAnimationSequence();
    UAnimDataModel* AnimDataModel = AnimSequence ? AnimSequence->GetDataModel() : nullptr;
    if (!AnimSequence || !AnimDataModel) return;
    const float playLength = AnimDataModel->PlayLength;
    const int   frameCount = AnimDataModel->NumberOfFrames;

    // --- Animation Selection Combo ---
    TMap<FString, UAnimationAsset*>& AnimAssets = FManagerFBX::GetAnimationAssets();
    static TArray<const char*> ItemPtrs;
    ItemPtrs.Empty(AnimAssets.Num());
    for (const auto& Asset : AnimAssets) {
        ItemPtrs.Add(*Asset.Key);
    }
    static int CurrentIndex = 0;
    if (ImGui::Combo("Animation##combo", &CurrentIndex, ItemPtrs.GetData(), ItemPtrs.Num())) {
        FString Key(ItemPtrs[CurrentIndex]);
        if (UAnimationAsset* Asset = AnimAssets[Key]) {
            if (UAnimSequence* Seq = Cast<UAnimSequence>(Asset)) {
                MyInst->SetAnimationSequence(Seq, true, MyInst->GetPlayRate());
                MyInst->SetPlayingState(true);
                MyInst->SetCurrentTime(0.0f);
            }
        }
    }

    // --- Playback Controls ---
    ImGui::BeginGroup();
    if (ImGui::Button(MyInst->IsPlaying() ? "Pause" : "Play")) {
        MyInst->SetPlayingState(!MyInst->IsPlaying());
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        MyInst->SetPlayingState(false);
        MyInst->SetCurrentTime(0.0f);
    }
    ImGui::SameLine();
    bool bLoop = MyInst->IsLooping();
    if (ImGui::Checkbox("Loop", &bLoop)) {
        MyInst->SetLooping(bLoop);
    }
    ImGui::SameLine();
    float rate = MyInst->GetPlayRate();
    if (ImGui::InputFloat("PlayRate", &rate, 0.1f, 1.0f, "%.2f")) {
        MyInst->SetPlayRate(rate);
    }
    ImGui::EndGroup();

    // --- Time Slider ---
    float currentTime = MyInst->GetCurrentTime();
    if (ImGui::SliderFloat("Time", &currentTime, 0.0f, playLength, "%.3f s")) {
        MyInst->SetCurrentTime(currentTime);
    }
    int32_t currentFrame = FMath::FloorToInt32((currentTime / playLength) * frameCount);
    int32_t startFrame = 0;
    int32_t endFrame = frameCount;
    static bool transformOpen = true;
    TArray<int> RemoveNotifiesIndex;

    // Open on Add Notify Button
    if (ImGui::Button("Add Notify")) {
        ImGui::OpenPopup("AddNotifyPopup");
    }
    if (ImGui::BeginPopup("AddNotifyPopup")) {
        static int newTrack = 1;
        static int newStartFrame = 0;
        static int newEndFrame = 0;
        static char newName[64] = "Notify";
        static int modeIndex = 0;
        const char* modes[] = { "Single", "State" };

        ImGui::Text("Add Notify");
        ImGui::Separator();
        ImGui::InputInt("Track", &newTrack);
        ImGui::InputInt("Start Frame", &newStartFrame);
        ImGui::Combo("Mode", &modeIndex, modes, IM_ARRAYSIZE(modes));
        if (modeIndex == 1) {
            ImGui::InputInt("End Frame", &newEndFrame);
        }
        ImGui::InputText("Name", newName, IM_ARRAYSIZE(newName));
        if (ImGui::Button("Add")) {
            FName notifyFName(newName);
            float startTime = ((float)newStartFrame / frameCount) * playLength;
            FAnimNotifyEvent newEvent = (modeIndex == 0)
                ? FAnimNotifyEvent(startTime, 0.f, notifyFName)
                : FAnimNotifyEvent(startTime, 0.f, notifyFName, ENotifyMode::State, ENotifyState::Begin);
            newEvent.SetTrackNum(newTrack);
            newEvent.UpdateTriggerFrame(playLength, frameCount);
            newEvent.UpdateTriggerTime(playLength, frameCount);
            if (modeIndex == 1) {
                newEvent.TriggerEndFrame = newEndFrame;
                newEvent.UpdateTriggerEndTime(playLength, frameCount);
                newEvent.UpdateTriggerEndFrame(playLength, frameCount);
            }
            AnimSequence->Notifies.Add(newEvent);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // --- Sequencer UI ---
    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, ImVec2(0, 0),
        ImGuiNeoSequencerFlags_EnableSelection |
        ImGuiNeoSequencerFlags_Selection_EnableDragging |
        ImGuiNeoSequencerFlags_Selection_EnableDeletion))
    {
        if (ImGui::BeginNeoGroup("Notifies", &transformOpen))
        {
            // State notifies first
            for (int track = 1; track <= AnimSequence->GetNotifyTrackCount(); ++track)
            {
                char timelineLabel[64];
                snprintf(timelineLabel, sizeof(timelineLabel), "%d##Track%d", track, track);
                if (ImGui::BeginNeoTimelineEx(timelineLabel))
                {
                    // State notifies
                    for (int j = 0; j < AnimSequence->Notifies.Num(); ++j)
                    {
                        FAnimNotifyEvent& Notify = AnimSequence->Notifies[j];
                        if (Notify.TrackNum != track || Notify.NotifyMode != ENotifyMode::State) continue;
                        Notify.UpdateTriggerFrame(playLength, frameCount);
                        Notify.UpdateTriggerEndFrame(playLength, frameCount);

                        ImGui::ShowNotifyState(*Notify.NotifyName.ToString(), Notify.TriggerFrame, Notify.TriggerEndFrame);
                        ImGui::NeoKeyframe(&Notify.TriggerFrame);
                        ImGui::NeoKeyframe(&Notify.TriggerEndFrame);
                        Notify.UpdateTriggerTime(playLength, frameCount);
                        Notify.UpdateTriggerEndTime(playLength, frameCount);

                        if (ImGui::IsNeoKeyframeSelected()) RemoveNotifiesIndex.Add(j);
                    }
                    // Single keyframe notifies
                    for (int j = 0; j < AnimSequence->Notifies.Num(); ++j)
                    {
                        FAnimNotifyEvent& Notify = AnimSequence->Notifies[j];
                        if (Notify.TrackNum != track || Notify.NotifyMode != ENotifyMode::Single) continue;
                        Notify.UpdateTriggerFrame(playLength, frameCount);
                        ImGui::NeoKeyframe(&Notify.TriggerFrame);
                        Notify.UpdateTriggerTime(playLength, frameCount);
                        if (ImGui::IsNeoKeyframeSelected()) RemoveNotifiesIndex.Add(j);
                    }
                    ImGui::EndNeoTimeLine();
                }
            }
            ImGui::EndNeoGroup();
        }
        RemoveNotifiesIndex.Sort();
        for (int idx = RemoveNotifiesIndex.Num() - 1; idx >= 0; --idx)
        {
            AnimSequence->Notifies.RemoveAt(RemoveNotifiesIndex[idx]);
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
