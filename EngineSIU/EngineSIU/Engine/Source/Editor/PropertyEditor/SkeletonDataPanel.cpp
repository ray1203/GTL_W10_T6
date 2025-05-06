#include "SkeletonDataPanel.h"
#include "World/World.h"
#include "Classes/Actors/ASkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Engine/EditorEngine.h"
#include <functional>
#include "Core/Math/Quat.h"
#include "ImGUI/imgui.h"

void SkeletonDataPanel::Render()
{
    /* 패널 설정 */
    const float rightMargin = 0.0f;
    const float topMargin = 0.0f;

    ImGui::SetNextWindowPos(ImVec2(
        ImGui::GetIO().DisplaySize.x - Width,
        topMargin
    ), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(
        Width,
        Height
    ), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    /* 본 계층 구조 표시 시작 */
    ImGui::Begin("Skeleton Hierarchy", nullptr, flags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    /* 선택된 스켈레탈 메시 컴포넌트 가져오기 */
    AActor* SelectedActor = nullptr;

    for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
    {
        if (ASkeletalMeshActor* SkeletalActor = Cast<ASkeletalMeshActor>(Actor))
        {
            SelectedActor = SkeletalActor;
        }
    }

    if (!Engine || !SelectedActor)
    {
        ImGui::Text("No Skeletal Mesh Selected");
        ImGui::End();
        return;
    }

    USkeletalMeshComponent* SkeletalComp = SelectedActor->GetComponentByClass<USkeletalMeshComponent>();

    USkeletalMesh* SkeletalMesh = SkeletalComp->GetSkeletalMesh();

    if (!SkeletalMesh) 
    {
        ImGui::Text("No Skeletal Mesh Asset");
        ImGui::End();
        return;
    };

    Skeleton = SkeletalMesh->Skeleton;

    if (!Skeleton)
    {
        ImGui::Text("Invalid Skeleton");
        ImGui::End();
        return;
    }

    const FReferenceSkeleton& RefSkeleton = Skeleton->ReferenceSkeleton;

    /* 부모-자식 관계 맵 생성 */
    TMap<int32, TArray<int32>> ParentToChildren;
    for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.BoneInfo.Num(); ++BoneIdx)
    {
        ParentToChildren.Add(BoneIdx, TArray<int32>()); // 모든 본에 엔트리 생성
    }

    for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.BoneInfo.Num(); ++BoneIdx)
    {
        const int32 ParentIdx = RefSkeleton.BoneInfo[BoneIdx].ParentIndex;
        ParentToChildren.FindOrAdd(ParentIdx).Add(BoneIdx);
    }

    /* 계층 구조 표시 */
    ImGui::BeginChild("BoneTree", ImVec2(0, 0), true);

    // 재귀적 본 트리 생성 함수
    std::function<void(int32)> DrawBoneTree = [&](int32 BoneIndex)
        {
            const FBoneNode* Bone = &RefSkeleton.BoneInfo[BoneIndex];
            const TArray<int32> Children = *ParentToChildren.Find(BoneIndex);

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
            
            if (Children.Num() == 0) 
            {
                nodeFlags |= ImGuiTreeNodeFlags_Leaf;
            };

            bool isOpen = ImGui::TreeNodeEx(
                (*Bone->Name.ToString()),
                nodeFlags
            );

            if (ImGui::IsItemClicked())
            {
                Engine->SelectBone(Bone);
            }

            if (isOpen)
            {
                for (int32 ChildIndex : Children)
                {
                    DrawBoneTree(ChildIndex);
                }
                ImGui::TreePop();
            }
        };

    // 루트 본 표시 (ParentIndex == INDEX_NONE)
    const TArray<int32>& RootBones = *ParentToChildren.Find(INDEX_NONE);
    for (int32 RootIndex : RootBones)
    {
        DrawBoneTree(RootIndex);
    }

    ImGui::EndChild();
    ImGui::End();

    DrawBoneTransformPanel();
}

void SkeletonDataPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}

void SkeletonDataPanel::DrawBoneTransformPanel() const
{
    // 패널 위치/크기 설정
    const float panelWidth = 500.0f;
    const float panelHeight = 300.0f;

    ImGui::SetNextWindowPos(ImVec2(
        ImGui::GetIO().DisplaySize.x - Width,
        Height
    ), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(
        Width,
        350
    ), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Bone Properties", nullptr, flags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    // 선택된 본 정보 가져오기
    const FBoneNode* SelectedBone = Engine->GetSelectedBone();

    if (SelectedBone)
    {
        int BoneIndex = Skeleton->BoneNameToIndex[SelectedBone->Name];
        // 1. 본 기본 정보 표시
        ImGui::Text("Bone Name: %s", (*SelectedBone->Name.ToString()));
        ImGui::Separator();

        // 2. 트랜스폼 편집
        FMatrix LocalTransform = Skeleton->CurrentPose.LocalTransforms[BoneIndex];
        FMatrix GlobalTransform = Skeleton->CurrentPose.GlobalTransforms[BoneIndex];

        FVector LocalPos = LocalTransform.GetTranslationVector();
        FRotator LocalRot = LocalTransform.ToQuat().Rotator(); // 쿼터니언 → 로테이터
        FVector LocalScale = LocalTransform.GetScaleVector();

        FVector GlobalPos = GlobalTransform.GetTranslationVector();
        FRotator GlobalRot = GlobalTransform.ToQuat().Rotator();
        FVector GlobalScale = GlobalTransform.GetScaleVector();

        // 3. 디버그 정보 표시
        /* 로컬 변환 편집 */
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "[Local Transform]");
        ImGui::InputFloat3("Position##Local", &LocalPos.X);
        ImGui::InputFloat3("Rotation##Local", &LocalRot.Pitch);
        ImGui::InputFloat3("Scale##Local", &LocalScale.X);

        /* 글로벌 변환 표시 */
        ImGui::TextColored(ImVec4(1, 0, 1, 1), "[Global Transform]");
        ImGui::Text("Position: %.2f, %.2f, %.2f", GlobalPos.X, GlobalPos.Y, GlobalPos.Z);
        ImGui::Text("Rotation: P=%.1f Y=%.1f R=%.1f", GlobalRot.Pitch, GlobalRot.Yaw, GlobalRot.Roll);
        ImGui::Text("Scale: %.2f, %.2f, %.2f", GlobalScale.X, GlobalScale.Y, GlobalScale.Z);
        // 3. 변경 사항 적용
        //if (ImGui::Button("Apply Transform"))
        //{
        //    const_cast<FBoneNode*>(SelectedBone)->BindTransform = LocalTransform;
        //    //Skeleton->MarkPackageDirty();
        //}
    }
    else
    {
        ImGui::Text("No Bone Selected");
    }

    ImGui::End();
}
