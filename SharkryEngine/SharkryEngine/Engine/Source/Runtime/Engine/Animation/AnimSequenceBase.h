#pragma once
#include "AnimationAsset.h"
#include "AnimTypes.h"

class UAnimDataModel;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)

public:
    UAnimSequenceBase();
    ~UAnimSequenceBase() = default;

public:
    UAnimDataModel* GetDataModel() const;
    void SetDataModel(UAnimDataModel* InAnimDataModel) { AnimDataModel = InAnimDataModel; }
    UAnimDataModel* GetDataModel() { return AnimDataModel; }
    // 원래는 GetAnimationPose를 가상함수로 두려 했으나 DECLARE_CLASS를 위해 그냥 함수로 두었음
    // 가상함수는 Instance가 될 수 없기에
    virtual void GetAnimationPose(FPoseContext& OutAnimationPoseData, const FAnimExtractContext& ExtractionContext);    

    virtual void GetAnimNotifies(const float& StartTime, const float& DeltaTime,
        const bool bAllowLooping, TArray<FAnimNotifyEvent*>& OutNotifies);

    /** Animation notifies, sorted by time (earliest notification first). */
    UPROPERTY(TArray<struct FAnimNotifyEvent>, Notifies);

    void SetNotifyTrackCount(int InNotifyTrackCount) { NotifyTrackCount = InNotifyTrackCount; }
    int GetNotifyTrackCount() { return NotifyTrackCount; }

protected:
    UAnimDataModel* AnimDataModel;

    int NotifyTrackCount = 1; // 기본적으로 Notify Track 한개는 두도록 함 언리얼도 그렇고 1개는 있어야 한다고 보이기에
};

