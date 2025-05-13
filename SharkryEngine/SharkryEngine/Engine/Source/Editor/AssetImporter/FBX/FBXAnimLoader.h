#pragma once

#include "Define.h"
#include <fbxsdk.h>

class UAnimDataModel;
enum class ETransformChannel;

namespace FBX
{
    /// 애니메이션 FBX 파일을 파싱하여 내부 포맷으로 변환하는 로더
    class FBXAnimLoader
    {
    public:
        /// FBX 파일로부터 애니메이션을 파싱합니다 (스켈레톤 이름 기반으로 자동 연결)
        static void ParseFBXAnim(
            const FString& FBXFilePath,
            const FString& AnimParentFBXFilePath
        );

        /// FBX 애니메이션 커브 키를 추출하여 AnimDataModel에 기록합니다
        static void ParseFBXCurveKey(
            FbxNode* BoneNode,
            FbxAnimLayer* Layer,
            UAnimDataModel* AnimDataModel,
            const FString& PropertyName,
            ETransformChannel TransformChannel,
            const char* pChannel
        );


    private:
        // FBX SDK에서 사용되는 좌표 채널 이름 배열
        static const FString TranslationChannels[3];
        static const FString RotationChannels[3];
        static const FString ScalingChannels[3];
    };
}
