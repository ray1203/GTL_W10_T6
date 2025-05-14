
#include "FBXAnimLoader.h"

#include "Misc/FrameRate.h"
#include "UObject/ObjectFactory.h"
#include "FBXAnimLoader.h"

#include <filesystem>
#include <fstream>

#include "FBXManager.h"
#include "FBXSceneLoader.h"
#include "FLoaderFBX.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "AssetImporter/FPaths.h"
#include "UObject/NameTypes.h"

void FBX::FBXAnimLoader::ParseFBXAnim(const FString& FBXFilePath)
{
    FbxManager* SdkManager = FBXSceneLoader::GetSdkManager();
    FbxScene* Scene = FBXSceneLoader::GetScene();
    FbxImporter* Importer = FBXSceneLoader::GetImporter();

#if USE_WIDECHAR
    std::string FilepathStdString = FBXFilePath.ToAnsiString();
#else
    std::string FilepathStdString(*FBXFilePath);
#endif
    if (!Importer->Initialize(FilepathStdString.c_str(), -1, SdkManager->GetIOSettings())) return;
    if (!Importer->Import(Scene)) return;

    FbxAxisSystem TargetAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
    if (Scene->GetGlobalSettings().GetAxisSystem() != TargetAxisSystem)
        TargetAxisSystem.DeepConvertScene(Scene);

    FbxSystemUnit::m.ConvertScene(Scene);

    TArray<FbxNode*> BoneNodes;
    FbxNode* Root = Scene->GetRootNode();
    CollectSkeletonNodes(Root, BoneNodes);

    const FString BaseName = FPaths::GetBaseFilename(FBXFilePath);
    FString OutputDir = TEXT("Contents/Binary/Animation");
    std::filesystem::create_directories(std::string(OutputDir)); // 디렉터리 생성

    int NumStacks = Scene->GetSrcObjectCount<FbxAnimStack>();
    for (int i = 0; i < NumStacks; i++)
    {
        // Stack의 개수만큼 애니메이션의 클립이 있는 것임
        FbxAnimStack* Stack = Scene->GetSrcObject<FbxAnimStack>(i);
        Scene->SetCurrentAnimationStack(Stack);

        FString TakeName = (Stack->GetName());
        FString CleanedTakeName = TakeName.Replace(TEXT("|"), TEXT("_")); // 파일 시스템 안전 이름

        FString FullName = BaseName + TEXT("_") + CleanedTakeName;
        FWString BinPath = FWString((OutputDir + "/" + (FullName + TEXT(".anim.bin"))).ToWideString().c_str());

        if (UAnimSequence* Cached = FBX::FBXAnimLoader::LoadAnimFromBinary(BinPath))
        {
            Cached->SetAssetPath(FBXFilePath);
            FManagerFBX::AddAnimationAsset(FullName, Cached);
            UE_LOG(LogLevel::Display, TEXT("Animation loaded from binary: %s"), *FullName);
            continue;
        }

        //FString TakeName = FBXFilePath;

        // 엔진 내부 구조도 준비
        UAnimDataModel* AnimDataModel = FObjectFactory::ConstructObject<UAnimDataModel>(nullptr);
        // FrameRate 구하기
        FbxGlobalSettings& Globals = Scene->GetGlobalSettings();
        FbxTime::EMode TimeMode = Globals.GetTimeMode();
        AnimDataModel->FrameRate = FFrameRate(FbxTime::GetFrameRate(TimeMode), 1);
        // NubmerOfFrames 구하기, 커브가 들어가면 수정 필요할수도있음
        FbxTimeSpan Span = Stack->GetLocalTimeSpan();
        double DurationSeconds = Span.GetDuration().GetSecondDouble();
        AnimDataModel->NumberOfFrames = int32(FMath::FloorToDouble(DurationSeconds * AnimDataModel->FrameRate.AsDecimal() + 0.5f));
        // PlayLength 값 구하기
        AnimDataModel->PlayLength = AnimDataModel->NumberOfFrames / AnimDataModel->FrameRate.AsDecimal();


        AnimDataModel->NumberOfKeys = AnimDataModel->NumberOfFrames;

        // Stack 내부에서 레이어 꺼내서 사용
        // 일단은 단일 레이어로 가정하여 0번에서 가져다가 사용
        // 이후 다중 레이어 고려해야하고 그 땐 curveNode->GetCurveRecursive() 사용 고려해볼것
        // TODO 다중 레이어 대응
        FbxAnimLayer* Layer = Stack->GetMember<FbxAnimLayer>(0);

        // 미리구한 Bone 목록을 통해 프레임 정보에 접근

        for (FbxNode* BoneNode : BoneNodes)
        {
            // FBoneAnimationTrack 추출

            FBoneAnimationTrack AnimationTrack;
            AnimationTrack.Name = BoneNode->GetName();

            for (int j = 0; j < AnimDataModel->NumberOfKeys; ++j)
            {
                // 1) 시간 셋업
                FbxTime time;
                time.SetFrame(j, Scene->GetGlobalSettings().GetTimeMode());

                // 2) 로컬(Parent-relative) 변환 한 방에 평가
                FbxAMatrix localXform = BoneNode->EvaluateLocalTransform(time);

                // 3) Translation
                FbxVector4 t = localXform.GetT();
                AnimationTrack.InternalTrack.PosKeys.Add(
                    FVector(static_cast<float>(t[0]),
                        static_cast<float>(t[1]),
                        static_cast<float>(t[2])));

                // 4) Rotation (Quat)
                //    FBX 쿼터니언 (X,Y,Z,W) 순서와 언리얼 FQuat 생성자 순서가 같습니다.
                FbxQuaternion fq = localXform.GetQ();
                FQuat  uq(
                    static_cast<float>(fq[3]),
                    static_cast<float>(fq[0]),
                    static_cast<float>(fq[1]),
                    static_cast<float>(fq[2])
                );
                AnimationTrack.InternalTrack.RotKeys.Add(uq);

                // 5) Scale
                FbxVector4 s = localXform.GetS();
                AnimationTrack.InternalTrack.ScaleKeys.Add(
                    FVector(static_cast<float>(s[0]),
                        static_cast<float>(s[1]),
                        static_cast<float>(s[2])));
            }

            // 마지막에 트랙 추가
            AnimDataModel->BoneAnimationTracks.Add(AnimationTrack);

            // CurveKey 구하기
            // Translation
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, TranslationChannels[0], ETransformChannel::Translation, FBXSDK_CURVENODE_COMPONENT_X);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, TranslationChannels[1], ETransformChannel::Translation, FBXSDK_CURVENODE_COMPONENT_Y);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, TranslationChannels[2], ETransformChannel::Translation, FBXSDK_CURVENODE_COMPONENT_Z);

            // Rotation
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, RotationChannels[0], ETransformChannel::Rotation, FBXSDK_CURVENODE_COMPONENT_X);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, RotationChannels[1], ETransformChannel::Rotation, FBXSDK_CURVENODE_COMPONENT_Y);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, RotationChannels[2], ETransformChannel::Rotation, FBXSDK_CURVENODE_COMPONENT_Z);

            // Scale
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, ScalingChannels[0], ETransformChannel::Scaling, FBXSDK_CURVENODE_COMPONENT_X);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, ScalingChannels[1], ETransformChannel::Scaling, FBXSDK_CURVENODE_COMPONENT_Y);
            ParseFBXCurveKey(BoneNode, Layer, AnimDataModel, ScalingChannels[2], ETransformChannel::Scaling, FBXSDK_CURVENODE_COMPONENT_Z);

        }

        // 4) UAnimSequence 생성 및 DataModel 연결
        UAnimSequence* Sequence = FObjectFactory::ConstructObject<UAnimSequence>(nullptr);
        Sequence->SetDataModel(AnimDataModel);

        // 5) 매니저에 등록
        Sequence->SetAssetPath(FBXFilePath);
        FManagerFBX::AddAnimationAsset(FullName, Sequence);
        UE_LOG(LogLevel::Warning, "Animation ADD : %s", *FullName);

        /*FString FileName = FPaths::GetFileNameWithoutExtension(FBXFilePath);
        FString OutputDir = TEXT("Contents/Binary/Animation");
        std::filesystem::create_directories(std::string(OutputDir)); // 디렉터리 생성
        FString OutputPath = OutputDir + TEXT("/") + FileName + TEXT(".anim.bin");*/

        FBX::FBXAnimLoader::SaveAnimToBinary(BinPath, Sequence);

        //FBX::FBXAnimLoader::SaveAnimToBinary(BinPath, Sequence);
    }
}
const FString FBX::FBXAnimLoader::TranslationChannels[3] = {
    TEXT("Translation.X"),
    TEXT("Translation.Y"),
    TEXT("Translation.Z")
};

const FString FBX::FBXAnimLoader::RotationChannels[3] = {
    TEXT("Rotation.X"),
    TEXT("Rotation.Y"),
    TEXT("Rotation.Z")
};

const FString FBX::FBXAnimLoader::ScalingChannels[3] = {
    TEXT("Scaling.X"),
    TEXT("Scaling.Y"),
    TEXT("Scaling.Z")
};


void FBX::FBXAnimLoader::ParseFBXCurveKey(
    FbxNode* BoneNode,
    FbxAnimLayer* Layer,
    UAnimDataModel* AnimDataModel,
    const FString& PropertyName,
    ETransformChannel TransformChannel,
    const char* pChannel
) {
    FString BoneNameStr = BoneNode->GetName();
    // FAnimationCurveChannel 추가
    FAnimationCurveChannel AnimationCurveChannel;
    FString PropNameStr = BoneNameStr + PropertyName;
    AnimationCurveChannel.PropertyName = PropNameStr;
    FbxAnimCurve* AnimCurve = nullptr;

    switch (TransformChannel)
    {
    case ETransformChannel::Translation:
        AnimCurve = BoneNode->LclTranslation.GetCurve(Layer, pChannel, false);
        break;
    case ETransformChannel::Rotation:
        AnimCurve = BoneNode->LclRotation.GetCurve(Layer, pChannel, false);
        break;
    case ETransformChannel::Scaling:
        AnimCurve = BoneNode->LclScaling.GetCurve(Layer, pChannel, false);
        break;
    default:
        break;
    }

    if (AnimCurve == nullptr)
    {
        // Scaling이나 Translation은 생략하여 데이터 절약을 하는 FBX도 있으므로
        return;
    }

    for (int j = 0; j < AnimCurve->KeyGetCount(); j++)
    {
        FbxAnimCurveKey Key = AnimCurve->KeyGet(j);

        FAnimationCurveKey CurveKey;
        CurveKey.Time = Key.GetTime().GetSecondDouble();
        CurveKey.Value = Key.GetValue();

        switch (Key.GetInterpolation())
        {
        case FbxAnimCurveDef::eInterpolationConstant:
            CurveKey.InterpMode = EInterpMode::RCIM_Constant;
            break;
        case FbxAnimCurveDef::eInterpolationLinear:
            CurveKey.InterpMode = EInterpMode::RCIM_Linear;
            break;
        case FbxAnimCurveDef::eInterpolationCubic:
            CurveKey.InterpMode = EInterpMode::RCIM_Cubic;
            break;
        default:
            CurveKey.InterpMode = EInterpMode::RCIM_Linear;
            break;
        }

        CurveKey.ArriveTangent = AnimCurve->KeyGetLeftDerivative(j);
        CurveKey.LeaveTangent = AnimCurve->KeyGetRightDerivative(j);

        AnimationCurveChannel.Keys.Add(CurveKey);
    }

    AnimDataModel->CurveData.Channels.Add(AnimationCurveChannel);

}

void FBX::FBXAnimLoader::CollectSkeletonNodes(FbxNode* Node, TArray<FbxNode*>& OutBones)
{
    if (!Node) return;

    // 1) 이 노드가 Skeleton 속성을 가지고 있으면 배열에 추가
    FbxNodeAttribute* Attr = Node->GetNodeAttribute();
    if (Attr && Attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        OutBones.Add(Node);
    }

    // 2) 자식 노드들도 재귀 순회
    const int ChildCount = Node->GetChildCount();
    for (int i = 0; i < ChildCount; ++i)
    {
        CollectSkeletonNodes(Node->GetChild(i), OutBones);
    }
}
bool FBX::FBXAnimLoader::SaveAnimToBinary(const FWString& FilePath, const UAnimSequence* Sequence)
{
    if (!Sequence || !Sequence->GetDataModel()) return false;
    const UAnimDataModel* DataModel = Sequence->GetDataModel();

    std::ofstream OutFile(FilePath.c_str(), std::ios::binary);
    if (!OutFile.is_open()) return false;

    int32 FrameCount = DataModel->NumberOfFrames;
    OutFile.write(reinterpret_cast<const char*>(&FrameCount), sizeof(int32));
    OutFile.write(reinterpret_cast<const char*>(&DataModel->PlayLength), sizeof(float));
    OutFile.write(reinterpret_cast<const char*>(&DataModel->FrameRate.Numerator), sizeof(int32));
    OutFile.write(reinterpret_cast<const char*>(&DataModel->FrameRate.Denominator), sizeof(int32));

    int32 BoneTrackCount = DataModel->BoneAnimationTracks.Num();
    OutFile.write(reinterpret_cast<const char*>(&BoneTrackCount), sizeof(int32));
    for (const FBoneAnimationTrack& Track : DataModel->BoneAnimationTracks)
    {
        FString Name = Track.Name.ToString();
        int32 NameLen = Name.Len();
        OutFile.write(reinterpret_cast<const char*>(&NameLen), sizeof(int32));
        OutFile.write((*Name), NameLen);

        OutFile.write(reinterpret_cast<const char*>(Track.InternalTrack.PosKeys.GetData()), sizeof(FVector) * Track.InternalTrack.PosKeys.Num());
        OutFile.write(reinterpret_cast<const char*>(Track.InternalTrack.RotKeys.GetData()), sizeof(FQuat) * Track.InternalTrack.RotKeys.Num());
        OutFile.write(reinterpret_cast<const char*>(Track.InternalTrack.ScaleKeys.GetData()), sizeof(FVector) * Track.InternalTrack.ScaleKeys.Num());
    }

    OutFile.close();
    return true;
}

UAnimSequence* FBX::FBXAnimLoader::LoadAnimFromBinary(const FWString& FilePath)
{
    std::ifstream InFile(FilePath.c_str(), std::ios::binary);
    if (!InFile.is_open()) return nullptr;

    int32 FrameCount = 0;
    float PlayLength = 0.0f;
    int32 RateNum = 30, RateDen = 1;
    InFile.read(reinterpret_cast<char*>(&FrameCount), sizeof(int32));
    InFile.read(reinterpret_cast<char*>(&PlayLength), sizeof(float));
    InFile.read(reinterpret_cast<char*>(&RateNum), sizeof(int32));
    InFile.read(reinterpret_cast<char*>(&RateDen), sizeof(int32));

    int32 BoneTrackCount = 0;
    InFile.read(reinterpret_cast<char*>(&BoneTrackCount), sizeof(int32));

    UAnimDataModel* DataModel = FObjectFactory::ConstructObject<UAnimDataModel>(nullptr);
    DataModel->NumberOfFrames = FrameCount;
    DataModel->NumberOfKeys = FrameCount;
    DataModel->PlayLength = PlayLength;
    DataModel->FrameRate = FFrameRate(RateNum, RateDen);

    for (int32 i = 0; i < BoneTrackCount; ++i)
    {
        int32 NameLen = 0;
        InFile.read(reinterpret_cast<char*>(&NameLen), sizeof(int32));
        std::string NameBuf(NameLen, '\0');
        InFile.read(NameBuf.data(), NameLen);
        FString BoneName((NameBuf.c_str()));

        FBoneAnimationTrack Track;
        Track.Name = FName(BoneName);
        Track.InternalTrack.PosKeys.SetNum(FrameCount);
        Track.InternalTrack.RotKeys.SetNum(FrameCount);
        Track.InternalTrack.ScaleKeys.SetNum(FrameCount);

        InFile.read(reinterpret_cast<char*>(Track.InternalTrack.PosKeys.GetData()), sizeof(FVector) * FrameCount);
        InFile.read(reinterpret_cast<char*>(Track.InternalTrack.RotKeys.GetData()), sizeof(FQuat) * FrameCount);
        InFile.read(reinterpret_cast<char*>(Track.InternalTrack.ScaleKeys.GetData()), sizeof(FVector) * FrameCount);

        DataModel->BoneAnimationTracks.Add(Track);
    }

    InFile.close();
    UAnimSequence* Sequence = FObjectFactory::ConstructObject<UAnimSequence>(nullptr);
    Sequence->SetDataModel(DataModel);
    return Sequence;
}
