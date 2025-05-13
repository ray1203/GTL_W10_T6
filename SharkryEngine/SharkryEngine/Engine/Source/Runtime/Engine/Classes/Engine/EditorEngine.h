#pragma once
#include "Engine.h"
#include "Actors/Player.h"

/*
    Editor 모드에서 사용될 엔진.
    UEngine을 상속받아 Editor에서 사용 될 기능 구현.
    내부적으로 PIE, Editor World 두 가지 형태로 관리.
*/

class AActor;
class USceneComponent;
class UPrimitiveComponent;
struct FBoneNode;
class APlayerCharacter;

class UEditorEngine : public UEngine
{
    DECLARE_CLASS(UEditorEngine, UEngine)

public:
    UEditorEngine() = default;

    virtual void Init() override;
    virtual void Tick(float DeltaTime) override;
    bool TryQuit(bool& OutbIsSave) override;
    void Release() override;

    void LoadLevel(const FString& FilePath) const;
    // 현재 Level을 저장하는 경우 빈 FString을 넣으세요.
    void SaveLevel(const FString& FilePath = FString("")) const;
    void SaveConfig() const;
    
    UWorld* PIEWorld = nullptr;
    UWorld* EditorWorld = nullptr;
    UWorld* ViewerWorld = nullptr;

    void StartPIE();
    void EndPIE();

    void StartViewer();

    // 주석은 UE에서 사용하던 매개변수.
    FWorldContext& GetEditorWorldContext(/*bool bEnsureIsGWorld = false*/);
    FWorldContext* GetPIEWorldContext(/*int32 WorldPIEInstance = 0*/);

public:
    void SelectActor(AActor* InActor);

    void SelectBone(const FBoneNode* InBone);

    // 전달된 액터가 선택된 컴포넌트와 같다면 해제 
    void DeselectActor(AActor* InActor);
    void ClearActorSelection(); 
    
    bool CanSelectActor(const AActor* InActor) const;
    AActor* GetSelectedActor() const;

    const FBoneNode* GetSelectedBone() const;

    void HoverActor(AActor* InActor);

    void NewLevel();

    void SelectComponent(USceneComponent* InComponent) const;
    
    // 전달된 InComponent가 현재 선택된 컴포넌트와 같다면 선택 해제
    void DeselectComponent(USceneComponent* InComponent);
    void ClearComponentSelection(); 
    
    bool CanSelectComponent(const USceneComponent* InComponent) const;
    USceneComponent* GetSelectedComponent() const;

    void HoverComponent(USceneComponent* InComponent);

    // 뷰어에서 사용하는 함수
    AActor* GetViewerTargetActor() const;
   
    USceneComponent* GetViewerTargetComponent() const;
    
    const void SetViewerTargetActor(AActor* InActor);
   
    const void SetViewerTargetComponent(USceneComponent* InComponent);
    
public:
    UEditorPlayer* GetEditorPlayer() const;
    
private:
    UEditorPlayer* EditorPlayer = nullptr;
    // 에디터 시작 시 컨트롤러 즉시 주입을 위한 캐릭터 생성(테스트용)
    APlayerCharacter* PlayerCharacter = nullptr;

};



