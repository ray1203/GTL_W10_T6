#include "FLoaderFBX.h"
#include "Define.h"
#include "Math/Vector.h"
#include <functional>
#include <numeric>
#include <algorithm>
#include <fbxsdk/utils/fbxrootnodeutility.h>
#include <filesystem>

FLinearColor ConvertFbxColorToLinear(const FbxDouble3& Color)
{
    // FBX 색상은 보통 Linear 공간이지만, 감마 보정이 필요하면 여기서 추가
    return FLinearColor(
        static_cast<float>(Color[0]),
        static_cast<float>(Color[1]),
        static_cast<float>(Color[2]),
        1.0f // Alpha는 보통 별도로 처리됨
    );
}


struct FControlPointSkinningData
{
    struct FBoneInfluence
    {
        int32 BoneIndex = INDEX_NONE;
        float Weight = 0.0f;
        bool operator>(const FBoneInfluence& Other) const { return Weight > Other.Weight; }
    };
    TArray<FBoneInfluence> Influences;

    void NormalizeWeights(int32 MaxInfluences)
    {
        if (Influences.IsEmpty()) return;
        float TotalWeight = 0.0f;
        for (const auto& Influence : Influences) TotalWeight += Influence.Weight;

        if (TotalWeight > KINDA_SMALL_NUMBER) {
            for (auto& Influence : Influences) Influence.Weight /= TotalWeight;
        }
        else if (!Influences.IsEmpty()) {
            float EqualWeight = 1.0f / Influences.Num();
            for (auto& Influence : Influences) Influence.Weight = EqualWeight;
        }

        Influences.Sort(std::greater<FBoneInfluence>());

        if (Influences.Num() > MaxInfluences) {
            Influences.SetNum(MaxInfluences);
            TotalWeight = 0.0f;
            for (const auto& Influence : Influences) TotalWeight += Influence.Weight;
            if (TotalWeight > KINDA_SMALL_NUMBER) {
                for (auto& Influence : Influences) Influence.Weight /= TotalWeight;
            }
            else if (!Influences.IsEmpty()) {
                Influences[0].Weight = 1.0f;
                for (int32 i = 1; i < Influences.Num(); ++i) Influences[i].Weight = 0.0f;
            }
        }

        if (!Influences.IsEmpty()) {
            float CurrentSum = 0.0f;
            for (int32 i = 0; i < Influences.Num() - 1; ++i) CurrentSum += Influences[i].Weight;
            int32 LastIndex = Influences.Num() - 1;
            if (LastIndex >= 0) {
                Influences[LastIndex].Weight = FMath::Max(0.0f, 1.0f - CurrentSum);
            }
        }
    }
};


// --- 생성자 / 소멸자 ---
// FLoaderFBX::FLoaderFBX() - 헤더에서 기본 생성자 사용 가정
// {
//     // 멤버 변수 초기화 (포인터는 nullptr로) - 헤더에서 처리됨
// }

FLoaderFBX::~FLoaderFBX()
{
    ReleaseBuffers(); // 멤버 버퍼 해제
    ShutdownFBXSDK(); // SDK 종료
}

USkeletalMesh* FLoaderFBX::CreateSkeletalMesh(const FString& Path)
{
    return nullptr;
}

// --- SDK 관리 ---
bool FLoaderFBX::InitializeFBXSDK()
{
    if (SdkManager) return true;

    SdkManager = FbxManager::Create();
    if (!SdkManager) return false;

    FbxIOSettings* IOS = FbxIOSettings::Create(SdkManager, IOSROOT);
    SdkManager->SetIOSettings(IOS);

    Scene = FbxScene::Create(SdkManager, "ImportScene");
    if (!Scene) {
        ShutdownFBXSDK();
        return false;
    }
    return true;
}

void FLoaderFBX::ShutdownFBXSDK()
{
    if (Scene) {
        Scene->Destroy();
        Scene = nullptr;
    }
    if (SdkManager) {
        SdkManager->Destroy();
        SdkManager = nullptr;
    }
}

// --- 멤버 버퍼 해제 ---
void FLoaderFBX::ReleaseBuffers()
{
    if (DynamicVertexBuffer) { DynamicVertexBuffer->Release(); DynamicVertexBuffer = nullptr; }
    if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = nullptr; }
}

// --- 로드 함수 (멤버 변수 채우기) ---
bool FLoaderFBX::LoadFBXFile(const FString& Filepath, ID3D11Device* Device)
{
    if (!Device) return false;

    // 1. 기존 리소스 정리 (멤버 변수 초기화)
    ReleaseBuffers();
    BindPoseVertices.Empty();
    FinalIndices.Empty();
    Bones.Empty();
    BoneNameToIndexMap.Empty();
    Materials.Empty();
    MaterialToIndexMap.Empty();
    LoadedFBXFilePath.Empty();
    LoadedFBXFileDirectory.clear();
    ShutdownFBXSDK();

    // FBX 파일 디렉토리 저장 (멤버 변수 사용)
    std::error_code ec;
    // FString을 std::filesystem::path로 변환 (FString이 char* 또는 wchar_t* 제공 가정)
    std::filesystem::path fsPath(*Filepath); // FString::operator*() 사용 가정
    std::filesystem::path parentPath = fsPath.parent_path();
    if (!parentPath.empty()) {
        LoadedFBXFileDirectory = parentPath.wstring().c_str(); // wstring으로 변환 후 FString 생성 가정
    }
    else {
        LoadedFBXFileDirectory.clear();
    }
    LoadedFBXFilePath = Filepath;


    // 2. SDK 초기화
    if (!InitializeFBXSDK()) return false;

    // 3. Importer 생성
    FbxImporter* Importer = FbxImporter::Create(SdkManager, "");
    if (!Importer) { ShutdownFBXSDK(); return false; }

    // 4. Importer 초기화 (FString -> std::string 변환 가정)
    std::string FilepathStdString(*Filepath); // FString::operator*() 또는 유사 기능 사용 가정
    bool bInit = Importer->Initialize(FilepathStdString.c_str(), -1, SdkManager->GetIOSettings());
    if (!bInit) {
        // 실패 시 에러 메시지 확인 가능: Importer->GetStatus().GetErrorString()
        Importer->Destroy();
        ShutdownFBXSDK();
        return false;
    }

    // 5. 씬 임포트
    if (!Importer->Import(Scene)) {
        Importer->Destroy();
        ShutdownFBXSDK();
        return false;
    }
    Importer->Destroy();

    // 6. 축 변환
    FbxAxisSystem TargetAxisSystem = FbxAxisSystem::DirectX;
    if (Scene->GetGlobalSettings().GetAxisSystem() != TargetAxisSystem) {
        TargetAxisSystem.ConvertScene(Scene);
    }

    // 7. 단위 변환 (예: cm 사용)
    FbxSystemUnit::cm.ConvertScene(Scene);

    // 8. FBX 루트 노드 제거 (선택 사항)
    FbxRootNodeUtility::RemoveAllFbxRoots(Scene);

    // 9. 지오메트리 삼각화
    FbxGeometryConverter GeometryConverter(SdkManager);
    GeometryConverter.Triangulate(Scene, true);

    // 10. 씬 처리 -> 멤버 변수 채우기
    if (!ProcessScene()) {
        ShutdownFBXSDK();
        return false;
    }

    // 11. DirectX 버퍼 생성 -> 멤버 변수(버퍼 포인터) 채우기
    if (!CreateBuffers(Device)) {
        ReleaseBuffers();
        ShutdownFBXSDK();
        return false;
    }

    // 12. SDK 정리 (로딩 완료 후)
    ShutdownFBXSDK();

    return true;
}

// --- 씬 처리 (멤버 변수 채우기) ---
bool FLoaderFBX::ProcessScene()
{
    FbxNode* RootNode = Scene->GetRootNode();
    if (!RootNode) return false;

    bool bMeshFoundAndProcessed = false;

    // 멤버 변수 초기화
    MaterialToIndexMap.Empty();
    Materials.Empty();
    BindPoseVertices.Empty();
    FinalIndices.Empty();
    Bones.Empty();
    BoneNameToIndexMap.Empty();

    // 루트 노드부터 재귀적으로 탐색 시작
    ProcessNodeRecursive(RootNode);

    // 멤버 BindPoseVertices가 채워졌는지 확인
    if (!BindPoseVertices.IsEmpty()) {
        bMeshFoundAndProcessed = true;
    }
    else {
        return false; // 처리할 메시 없음
    }

    // 스켈레톤 계층 구조 처리 (멤버 Bones 사용)
    if (bMeshFoundAndProcessed && !Bones.IsEmpty()) {
        if (!ProcessSkeletonHierarchy(RootNode)) {
            // 계층 구조 처리 실패 경고 (선택 사항)
        }
        CalculateInitialLocalTransforms(); // 멤버 함수 호출
        UpdateWorldTransforms();           // 멤버 함수 호출 (이름 변경 전 버전 사용)
    }

    return bMeshFoundAndProcessed;
}

// --- 재귀적 노드 탐색 및 처리 (멤버 변수 채움) ---
void FLoaderFBX::ProcessNodeRecursive(FbxNode* Node)
{
    if (!Node || !BindPoseVertices.IsEmpty()) return; // 이미 메시 찾았으면 중단

    FbxMesh* Mesh = Node->GetMesh();
    if (Mesh) {
        // 재질 처리 (멤버 Materials, MaterialToIndexMap 사용)
        int MaterialCount = Node->GetMaterialCount();
        for (int matIdx = 0; matIdx < MaterialCount; ++matIdx) {
            FbxSurfaceMaterial* FbxMat = Node->GetMaterial(matIdx);
            if (FbxMat && !MaterialToIndexMap.Contains(FbxMat)) { // 멤버 맵 사용
                FBX::FFbxMaterialInfo NewMatInfo = ProcessMaterial(FbxMat); // 멤버 Materials 채움
                int32 MaterialIndex = Materials.Add(NewMatInfo); // 멤버 배열 사용
                MaterialToIndexMap.Add(FbxMat, MaterialIndex); // 멤버 맵 사용
            }
        }

        // 메시 처리 (멤버 BindPoseVertices, FinalIndices, Bones 등 채움)
        if (ProcessMesh(Mesh)) {
            // 메시 처리 성공 시 탐색 중단
            return;
        }
    }

    // 자식 노드 탐색 (아직 메시 못 찾았으면)
    if (BindPoseVertices.IsEmpty()) {
        for (int i = 0; i < Node->GetChildCount(); ++i) {
            ProcessNodeRecursive(Node->GetChild(i));
            if (!BindPoseVertices.IsEmpty()) return; // 자식에서 찾았으면 중단
        }
    }
}


// --- 메시 처리 (멤버 변수 채우기) ---
bool FLoaderFBX::ProcessMesh(FbxMesh* Mesh)
{
    if (!Mesh) return false;

    const int32 ControlPointCount = Mesh->GetControlPointsCount();
    if (ControlPointCount <= 0) return false;

    const int32 PolygonCount = Mesh->GetPolygonCount();
    if (PolygonCount <= 0) return false;
    const int32 PolygonVertexCount = Mesh->GetPolygonVertexCount();
    if (PolygonVertexCount <= 0) return false;
    if (PolygonVertexCount != PolygonCount * 3) {
        // 삼각화 안 된 경우 경고 (선택 사항)
    }

    // 1. 제어점 위치 (로컬 변수)
    TArray<FVector> ControlPointPositions;
    ControlPointPositions.Reserve(ControlPointCount);
    FbxVector4* FbxControlPoints = Mesh->GetControlPoints();
    for (int32 i = 0; i < ControlPointCount; ++i) {
        ControlPointPositions.Add(ConvertFbxPosition(FbxControlPoints[i]));
    }

    // 2. 폴리곤 정점 인덱스 (로컬 변수)
    TArray<int32> PolygonVertexIndices;
    PolygonVertexIndices.Reserve(PolygonVertexCount);
    int* FbxPolygonVertices = Mesh->GetPolygonVertices();
    for (int32 i = 0; i < PolygonVertexCount; ++i) {
        PolygonVertexIndices.Add(FbxPolygonVertices[i]);
    }

    // 3. 노멀 데이터 (로컬 변수)
    TArray<FVector> ControlPointNormals;
    TArray<FVector> PolygonVertexNormals;
    FbxGeometryElementNormal* NormalElement = Mesh->GetElementNormal(0);
    if (NormalElement) {
        GetElementData<FbxGeometryElementNormal, FVector>(
            Mesh, NormalElement, ControlPointCount, PolygonVertexCount, PolygonVertexIndices,
            ControlPointNormals, PolygonVertexNormals
        );
    }

    // 4. UV 데이터 (로컬 변수)
    TArray<FVector2D> ControlPointUVs;
    TArray<FVector2D> PolygonVertexUVs;
    FbxGeometryElementUV* UVElement = Mesh->GetElementUV(0);
    if (UVElement) {
        GetElementData<FbxGeometryElementUV, FVector2D>(
            Mesh, UVElement, ControlPointCount, PolygonVertexCount, PolygonVertexIndices,
            ControlPointUVs, PolygonVertexUVs
        );
    }

    // 5. 스키닝 데이터 추출 (멤버 Bones, BoneNameToIndexMap 채움)
    TArray<FControlPointSkinningData> CpSkinData;
    CpSkinData.SetNum(ControlPointCount);
    ExtractSkinningData(Mesh, CpSkinData); // 멤버 변수 사용

    // 6. 가중치 정규화 (로컬 CpSkinData 수정)
    for (int32 i = 0; i < ControlPointCount; ++i) {
        if (CpSkinData.IsValidIndex(i)) {
            CpSkinData[i].NormalizeWeights(MAX_BONE_INFLUENCES);
        }
    }

    // 7. 최종 정점/인덱스 생성 (멤버 BindPoseVertices, FinalIndices 채움)
    FinalizeVertexData(
        ControlPointPositions, PolygonVertexIndices,
        ControlPointNormals, PolygonVertexNormals, NormalElement,
        ControlPointUVs, PolygonVertexUVs, UVElement,
        CpSkinData
    ); // 멤버 변수 사용

    // 멤버 변수가 채워졌는지 확인
    return !BindPoseVertices.IsEmpty() && !FinalIndices.IsEmpty();
}

// --- 스키닝 데이터 추출 (멤버 Bones, BoneNameToIndexMap 채움) ---
void FLoaderFBX::ExtractSkinningData(FbxMesh* Mesh, TArray<FControlPointSkinningData>& OutCpSkinData)
{
    const int32 ControlPointCount = Mesh->GetControlPointsCount();
    if (ControlPointCount <= 0) return;

    const int32 DeformerCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
    if (DeformerCount == 0) return;

    for (int32 DeformerIndex = 0; DeformerIndex < DeformerCount; ++DeformerIndex) {
        FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(DeformerIndex, FbxDeformer::eSkin));
        if (!Skin) continue;

        const int32 ClusterCount = Skin->GetClusterCount();
        for (int32 ClusterIndex = 0; ClusterIndex < ClusterCount; ++ClusterIndex) {
            FbxCluster* Cluster = Skin->GetCluster(ClusterIndex);
            FbxNode* BoneNode = Cluster->GetLink();
            if (!BoneNode) continue;

            // FName 생성 (FString 생성자 사용 가정)
            FName BoneName = FName(BoneNode->GetName());
            int32 CurrentBoneIndex = INDEX_NONE;

            // 멤버 BoneNameToIndexMap에서 검색
            int32* FoundIndexPtr = BoneNameToIndexMap.Find(BoneName);
            if (FoundIndexPtr == nullptr) {
                // 멤버 Bones 배열에 추가
                CurrentBoneIndex = Bones.Emplace();
                if (!Bones.IsValidIndex(CurrentBoneIndex)) continue;

                FBX::FBoneInfo& NewBone = Bones[CurrentBoneIndex]; // 멤버 배열 참조
                NewBone.Name = BoneName;
                NewBone.ParentIndex = INDEX_NONE;

                FbxAMatrix ClusterLinkGlobalInitMatrix;
                Cluster->GetTransformLinkMatrix(ClusterLinkGlobalInitMatrix); // 본 월드 변환 (바인드 시점)

                FbxAMatrix InvBindPoseFbx = ClusterLinkGlobalInitMatrix.Inverse();
                NewBone.InverseBindPoseMatrix = FMatrix::ConvertFbxAMatrixToFMatrix(InvBindPoseFbx);
                NewBone.BindPoseMatrix = FMatrix::ConvertFbxAMatrixToFMatrix(ClusterLinkGlobalInitMatrix);

                NewBone.CurrentLocalMatrix = FMatrix::Identity;
                NewBone.CurrentWorldTransform = NewBone.BindPoseMatrix;

                // 멤버 BoneNameToIndexMap에 추가
                BoneNameToIndexMap.Add(BoneName, CurrentBoneIndex);
            }
            else {
                CurrentBoneIndex = *FoundIndexPtr;
                if (Bones.IsValidIndex(CurrentBoneIndex) && Bones[CurrentBoneIndex].CurrentWorldTransform == FMatrix::Identity) {
                    Bones[CurrentBoneIndex].CurrentWorldTransform = Bones[CurrentBoneIndex].BindPoseMatrix;
                }
            }

            // 영향력 정보 처리
            int* ControlPointIndices = Cluster->GetControlPointIndices();
            double* ControlPointWeights = Cluster->GetControlPointWeights();
            const int32 InfluenceCount = Cluster->GetControlPointIndicesCount();
            for (int32 InfluenceIndex = 0; InfluenceIndex < InfluenceCount; ++InfluenceIndex) {
                int32 CPIndex = ControlPointIndices[InfluenceIndex];
                float Weight = static_cast<float>(ControlPointWeights[InfluenceIndex]);
                if (OutCpSkinData.IsValidIndex(CPIndex) && Weight > KINDA_SMALL_NUMBER && CurrentBoneIndex != INDEX_NONE) {
                    OutCpSkinData[CPIndex].Influences.Add({ CurrentBoneIndex, Weight });
                }
            }
        }
    }
}

// --- 최종 정점 데이터 생성 (멤버 BindPoseVertices, FinalIndices 채움) ---
void FLoaderFBX::FinalizeVertexData(
    const TArray<FVector>& ControlPointPositions,
    const TArray<int32>& PolygonVertexIndices,
    const TArray<FVector>& ControlPointNormals,
    const TArray<FVector>& PolygonVertexNormals,
    const FbxGeometryElementNormal* NormalElement,
    const TArray<FVector2D>& ControlPointUVs,
    const TArray<FVector2D>& PolygonVertexUVs,
    const FbxGeometryElementUV* UVElement,
    const TArray<struct FControlPointSkinningData>& CpSkinData)
{
    BindPoseVertices.Empty(); // 멤버 변수 초기화
    FinalIndices.Empty();     // 멤버 변수 초기화
    TMap<FBX::FMeshVertex, uint32> UniqueVertices; // 로컬 맵

    const int32 TotalPolygonVertices = PolygonVertexIndices.Num();
    FinalIndices.Reserve(TotalPolygonVertices); // 멤버 인덱스 배열 크기 예약

    for (int32 PolyVertIndex = 0; PolyVertIndex < TotalPolygonVertices; ++PolyVertIndex) {
        const int32 ControlPointIndex = PolygonVertexIndices[PolyVertIndex];
        if (!ControlPointPositions.IsValidIndex(ControlPointIndex)) continue;

        FBX::FMeshVertex CurrentVertex = {};

        // 1. 위치
        CurrentVertex.Position = ControlPointPositions[ControlPointIndex];

        // 2. 노멀
        if (NormalElement) {
            if (NormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
                if (ControlPointNormals.IsValidIndex(ControlPointIndex)) CurrentVertex.Normal = ControlPointNormals[ControlPointIndex];
                else CurrentVertex.Normal = FVector(0.f, 0.f, 1.f);
            }
            else {
                int32 SourceIndex = PolyVertIndex;
                if (NormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect && NormalElement->GetIndexArray().GetCount() > PolyVertIndex)
                    SourceIndex = NormalElement->GetIndexArray().GetAt(PolyVertIndex);
                else if (NormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) SourceIndex = -1;

                if (PolygonVertexNormals.IsValidIndex(SourceIndex)) CurrentVertex.Normal = PolygonVertexNormals[SourceIndex];
                else CurrentVertex.Normal = FVector(0.f, 0.f, 1.f);
            }
        }
        else { CurrentVertex.Normal = FVector(0.f, 0.f, 1.f); }
        CurrentVertex.Normal.Normalize();

        // 3. UV
        if (UVElement) {
            if (UVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
                if (ControlPointUVs.IsValidIndex(ControlPointIndex)) CurrentVertex.TexCoord = ControlPointUVs[ControlPointIndex];
                else CurrentVertex.TexCoord = FVector2D(0.f, 0.f);
            }
            else {
                int32 SourceIndex = PolyVertIndex;
                if (UVElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect && UVElement->GetIndexArray().GetCount() > PolyVertIndex)
                    SourceIndex = UVElement->GetIndexArray().GetAt(PolyVertIndex);
                else if (UVElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) SourceIndex = -1;

                if (PolygonVertexUVs.IsValidIndex(SourceIndex)) CurrentVertex.TexCoord = PolygonVertexUVs[SourceIndex];
                else CurrentVertex.TexCoord = FVector2D(0.f, 0.f);
            }
        }
        else { CurrentVertex.TexCoord = FVector2D(0.f, 0.f); }

        // 4. 스키닝 데이터
        CurrentVertex.BoneIndices[0] = 0; CurrentVertex.BoneWeights[0] = 1.0f;
        for (int i = 1; i < MAX_BONE_INFLUENCES; ++i) { CurrentVertex.BoneIndices[i] = 0; CurrentVertex.BoneWeights[i] = 0.0f; }
        if (CpSkinData.IsValidIndex(ControlPointIndex)) {
            const auto& Influences = CpSkinData[ControlPointIndex].Influences;
            if (!Influences.IsEmpty()) {
                for (int32 i = 0; i < Influences.Num() && i < MAX_BONE_INFLUENCES; ++i) {
                    CurrentVertex.BoneIndices[i] = Influences[i].BoneIndex;
                    CurrentVertex.BoneWeights[i] = Influences[i].Weight;
                }
                // 사용되지 않은 슬롯은 0으로 유지
                for (int32 i = Influences.Num(); i < MAX_BONE_INFLUENCES; ++i) {
                    CurrentVertex.BoneIndices[i] = 0;
                    CurrentVertex.BoneWeights[i] = 0.0f;
                }
            }
        }

        // 5. 중복 정점 확인 및 추가
        uint32* FoundIndexPtr = UniqueVertices.Find(CurrentVertex);
        if (FoundIndexPtr != nullptr) {
            FinalIndices.Add(*FoundIndexPtr); // 멤버 배열에 추가
        }
        else {
            uint32 NewIndex = static_cast<uint32>(BindPoseVertices.Add(CurrentVertex)); // 멤버 배열에 추가
            UniqueVertices.Add(CurrentVertex, NewIndex);
            FinalIndices.Add(NewIndex); // 멤버 배열에 추가
        }
    }
}


// --- 스켈레톤 계층 구조 처리 (멤버 Bones 수정) ---
bool FLoaderFBX::ProcessSkeletonHierarchy(FbxNode* RootNode)
{
    if (!RootNode || Bones.IsEmpty()) return Bones.IsEmpty(); // 멤버 변수 사용

    bool bHierarchySetCorrectly = true;
    for (FBX::FBoneInfo& Bone : Bones) Bone.ParentIndex = INDEX_NONE; // 멤버 변수 사용

    for (int i = 0; i < RootNode->GetChildCount(); ++i) {
        ProcessSkeletonNodeRecursive(RootNode->GetChild(i), INDEX_NONE);
    }

    int32 RootBoneCount = 0;
    bool bFoundInvalidParent = false;
    for (int32 i = 0; i < Bones.Num(); ++i) { // 멤버 변수 사용
        if (Bones[i].ParentIndex == INDEX_NONE) RootBoneCount++; // 멤버 변수 사용
        else if (!Bones.IsValidIndex(Bones[i].ParentIndex)) { // 멤버 변수 사용
            Bones[i].ParentIndex = INDEX_NONE; // 멤버 변수 사용
            bFoundInvalidParent = true;
            bHierarchySetCorrectly = false;
        }
    }
    if (RootBoneCount == 0 && Bones.Num() > 0) bHierarchySetCorrectly = false; // 멤버 변수 사용
    // if (bFoundInvalidParent) { /* 경고 로그 */ }

    // CalculateInitialLocalTransforms, UpdateWorldTransforms는 ProcessScene에서 호출됨

    return bHierarchySetCorrectly;
}

// --- 스켈레톤 노드 재귀 처리 (멤버 Bones, BoneNameToIndexMap 사용) ---
void FLoaderFBX::ProcessSkeletonNodeRecursive(FbxNode* Node, int32 CurrentParentBoneIndex)
{
    if (!Node) return;

    FName NodeName = FName(Node->GetName()); // FString 생성자 사용 가정
    int32 ThisNodeBoneIndex = INDEX_NONE;

    int32* FoundIndexPtr = BoneNameToIndexMap.Find(NodeName); // 멤버 맵 사용
    if (FoundIndexPtr != nullptr) {
        ThisNodeBoneIndex = *FoundIndexPtr;
        if (Bones.IsValidIndex(ThisNodeBoneIndex)) { // 멤버 배열 사용
            if (Bones[ThisNodeBoneIndex].ParentIndex == INDEX_NONE) { // 멤버 배열 사용
                Bones[ThisNodeBoneIndex].ParentIndex = CurrentParentBoneIndex; // 멤버 배열 사용
            }
        }
        else { ThisNodeBoneIndex = INDEX_NONE; }
    }

    int32 ParentIndexForChildren = (ThisNodeBoneIndex != INDEX_NONE) ? ThisNodeBoneIndex : CurrentParentBoneIndex;
    for (int32 i = 0; i < Node->GetChildCount(); ++i) {
        ProcessSkeletonNodeRecursive(Node->GetChild(i), ParentIndexForChildren);
    }
}


// --- 초기 로컬 변환 계산 (멤버 Bones 수정) ---
void FLoaderFBX::CalculateInitialLocalTransforms()
{
    if (Bones.IsEmpty()) return; // 멤버 변수 사용
    for (int32 i = 0; i < Bones.Num(); ++i) { // 멤버 변수 사용
        FBX::FBoneInfo& CurrentBone = Bones[i]; // 멤버 변수 사용
        const FMatrix& WorldBindPose = CurrentBone.BindPoseMatrix;
        int32 ParentIdx = CurrentBone.ParentIndex;
        if (ParentIdx != INDEX_NONE && Bones.IsValidIndex(ParentIdx)) { // 멤버 변수 사용
            const FBX::FBoneInfo& ParentBone = Bones[ParentIdx]; // 멤버 변수 사용
            const FMatrix& ParentWorldBindPose = ParentBone.BindPoseMatrix;
            FMatrix ParentInverseBindPose = FMatrix::Inverse(ParentWorldBindPose);
            if (FMath::Abs(ParentInverseBindPose.Determinant()) < SMALL_NUMBER) {
                ParentInverseBindPose = FMatrix::Identity;
            }
            CurrentBone.CurrentLocalMatrix = WorldBindPose * ParentInverseBindPose; // 멤버 변수 사용
        }
        else {
            CurrentBone.CurrentLocalMatrix = WorldBindPose; // 멤버 변수 사용
        }
    }
}


// --- DirectX 버퍼 생성 (멤버 버퍼 포인터 채움) ---
bool FLoaderFBX::CreateBuffers(ID3D11Device* Device)
{
    if (!Device || BindPoseVertices.IsEmpty() || FinalIndices.IsEmpty()) return false; // 멤버 변수 사용

    ReleaseBuffers(); // 기존 멤버 버퍼 해제

    HRESULT hr;

    // 동적 정점 버퍼
    D3D11_BUFFER_DESC DynVertexBufferDesc = {};
    DynVertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    DynVertexBufferDesc.ByteWidth = sizeof(FBX::FMeshVertex) * BindPoseVertices.Num(); // 멤버 변수 사용
    DynVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    DynVertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA VertexInitData = {};
    VertexInitData.pSysMem = BindPoseVertices.GetData(); // 멤버 변수 사용
    hr = Device->CreateBuffer(&DynVertexBufferDesc, &VertexInitData, &DynamicVertexBuffer); // 멤버 포인터 사용
    if (FAILED(hr)) return false;

    // 인덱스 버퍼
    D3D11_BUFFER_DESC IndexBufferDesc = {};
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.ByteWidth = sizeof(uint32) * FinalIndices.Num(); // 멤버 변수 사용
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA IndexInitData = {};
    IndexInitData.pSysMem = FinalIndices.GetData(); // 멤버 변수 사용
    hr = Device->CreateBuffer(&IndexBufferDesc, &IndexInitData, &IndexBuffer); // 멤버 포인터 사용
    if (FAILED(hr)) { ReleaseBuffers(); return false; }

    return true;
}

// --- 스키닝 업데이트 (멤버 변수 사용) ---
bool FLoaderFBX::UpdateAndApplySkinning(ID3D11DeviceContext* DeviceContext)
{
    // 참고: FinalBoneTransforms 파라미터는 현재 내부 로직에서 사용되지 않고,
    //       Bones[BoneIndex].CurrentWorldTransform (멤버 변수)를 직접 사용합니다.
    //       이 함수를 호출하기 전에 UpdateWorldTransforms()가 호출되어야 합니다.

    if (!DeviceContext || !DynamicVertexBuffer || BindPoseVertices.IsEmpty() || Bones.IsEmpty()) return false; // 멤버 변수 사용

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = DeviceContext->Map(DynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource); // 멤버 변수 사용
    if (FAILED(hr)) return false;

    FBX::FMeshVertex* SkinnedVertices = static_cast<FBX::FMeshVertex*>(MappedResource.pData);
    const int32 VertexCount = BindPoseVertices.Num(); // 멤버 변수 사용

    for (int32 i = 0; i < VertexCount; ++i) {
        const FBX::FMeshVertex& BindVertex = BindPoseVertices[i]; // 멤버 변수 사용
        FBX::FMeshVertex& SkinnedVertex = SkinnedVertices[i];
        bool bHasSignificantWeight = false;
        for (int j = 0; j < MAX_BONE_INFLUENCES; ++j) {
            if (BindVertex.BoneWeights[j] > KINDA_SMALL_NUMBER && Bones.IsValidIndex(static_cast<int32>(BindVertex.BoneIndices[j]))) { // 멤버 변수 사용
                bHasSignificantWeight = true;
                break;
            }
        }
        if (!bHasSignificantWeight) {
            SkinnedVertex = BindVertex; continue;
        }
        FVector SkinnedPosition = FVector::ZeroVector;
        FVector SkinnedNormal = FVector::ZeroVector;
        const FVector& BindPositionVec = BindVertex.Position;
        const FVector& BindNormalVec = BindVertex.Normal;
        for (int32 j = 0; j < MAX_BONE_INFLUENCES; ++j) {
            float Weight = BindVertex.BoneWeights[j];
            if (Weight <= KINDA_SMALL_NUMBER) continue;
            int32 BoneIndex = static_cast<int32>(BindVertex.BoneIndices[j]);
            if (!Bones.IsValidIndex(BoneIndex)) continue; // 멤버 변수 사용

            const FMatrix& InvBindPoseMat = Bones[BoneIndex].InverseBindPoseMatrix; // 멤버 변수 사용
            const FMatrix& FinalBoneTransformMat = Bones[BoneIndex].CurrentWorldTransform; // 멤버 변수 사용
            const FMatrix SkinMatrix = InvBindPoseMat * FinalBoneTransformMat;

            SkinnedPosition += SkinMatrix.TransformPosition(BindPositionVec) * Weight;
            FMatrix NormalMatrix = SkinMatrix; NormalMatrix.SetOrigin(FVector::ZeroVector);
            if (FMath::Abs(NormalMatrix.Determinant()) > SMALL_NUMBER) {
                FMatrix InvTranspose = FMatrix::Transpose(FMatrix::Inverse(NormalMatrix));
                SkinnedNormal += InvTranspose.TransformPosition(BindNormalVec) * Weight;
            }
            else { SkinnedNormal += BindNormalVec * Weight; }
        }
        SkinnedVertex.Position = SkinnedPosition;
        SkinnedVertex.Normal = SkinnedNormal.GetSafeNormal();
        SkinnedVertex.TexCoord = BindVertex.TexCoord;
        for (int k = 0; k < MAX_BONE_INFLUENCES; ++k) {
            SkinnedVertex.BoneIndices[k] = BindVertex.BoneIndices[k];
            SkinnedVertex.BoneWeights[k] = BindVertex.BoneWeights[k];
        }
    }
    DeviceContext->Unmap(DynamicVertexBuffer, 0); // 멤버 변수 사용
    return true;
}

// --- 렌더링 (멤버 변수 사용) ---
void FLoaderFBX::Render(ID3D11DeviceContext* DeviceContext)
{
    if (!DeviceContext || !DynamicVertexBuffer || !IndexBuffer || FinalIndices.IsEmpty()) return; // 멤버 변수 사용
    UINT Stride = sizeof(FBX::FMeshVertex); UINT Offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &DynamicVertexBuffer, &Stride, &Offset); // 멤버 변수 사용
    DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0); // 멤버 변수 사용
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->DrawIndexed(FinalIndices.Num(), 0, 0); // 멤버 변수 사용
}

// --- 매니저 관련 함수 스텁 (헤더에 정의됨) ---
// 실제 로직은 FManagerFBX 클래스에 있어야 함
FBX::FSkeletalMeshRenderData* FLoaderFBX::LoadSkeletalMeshAsset(const FString& PathFileName, ID3D11Device* Device)
{
    // 이 클래스는 로더 역할만 하므로, 매니저 기능은 여기서 구현하지 않음
    return nullptr;
}

FBX::FSkeletalMeshRenderData* FLoaderFBX::GetSkeletalMesh(const FString& PathFileName)
{
    // 이 클래스는 로더 역할만 하므로, 매니저 기능은 여기서 구현하지 않음
    return nullptr;
}

void FLoaderFBX::ClearAll()
{
     ReleaseBuffers();
    BindPoseVertices.Empty();
    FinalIndices.Empty();
    Bones.Empty();
    BoneNameToIndexMap.Empty();
    Materials.Empty();
    MaterialToIndexMap.Empty();
    LoadedFBXFilePath.Empty();
    LoadedFBXFileDirectory.clear();
    ShutdownFBXSDK(); // SDK도 종료
}


// --- 데이터 접근자 구현 (멤버 변수 사용) ---
int32 FLoaderFBX::GetBoneIndex(const FName& BoneName) const
{
    const int32* FoundIndex = BoneNameToIndexMap.Find(BoneName); // 멤버 맵 사용
    return FoundIndex ? *FoundIndex : INDEX_NONE;
}

const FBX::FBoneInfo* FLoaderFBX::GetBoneInfo(int32 BoneIndex) const
{
    if (Bones.IsValidIndex(BoneIndex)) return &Bones[BoneIndex]; // 멤버 배열 사용
    return nullptr;
}

void FLoaderFBX::UpdateWorldTransforms() // 헤더에 정의된 이름 사용
{
    if (Bones.IsEmpty()) return; // 멤버 변수 사용
    for (int32 i = 0; i < Bones.Num(); ++i) { // 멤버 변수 사용
        const FMatrix& CurrentLocalMatrix = Bones[i].CurrentLocalMatrix; // 멤버 변수 사용
        int32 ParentIdx = Bones[i].ParentIndex; // 멤버 변수 사용
        if (ParentIdx != INDEX_NONE && Bones.IsValidIndex(ParentIdx)) { // 멤버 변수 사용
            const FMatrix& ParentWorldTransform = Bones[ParentIdx].CurrentWorldTransform; // 멤버 변수 사용
            Bones[i].CurrentWorldTransform = CurrentLocalMatrix * ParentWorldTransform; // 멤버 변수 사용
        }
        else {
            Bones[i].CurrentWorldTransform = CurrentLocalMatrix; // 멤버 변수 사용
        }
    }
}

TArray<FName> FLoaderFBX::GetBoneNames() const
{
    TArray<FName> Names;
    if (!Bones.IsEmpty()) { // 멤버 변수 사용
        Names.Reserve(Bones.Num()); // 멤버 변수 사용
        for (const FBX::FBoneInfo& Bone : Bones) Names.Add(Bone.Name); // 멤버 변수 사용
    }
    return Names;
}

bool FLoaderFBX::SetBoneLocalMatrix(uint32_t BoneIndex, const FMatrix& NewLocalMatrix)
{
    if (!Bones.IsValidIndex(BoneIndex)) return false; // 멤버 변수 사용
    Bones[BoneIndex].CurrentLocalMatrix = NewLocalMatrix; // 멤버 변수 사용
    // 중요: 호출 후 UpdateWorldTransforms() 필요
    return true;
}

FMatrix FLoaderFBX::GetBoneLocalMatrix(uint32_t BoneIndex) const
{
    if (!Bones.IsValidIndex(BoneIndex)) return FMatrix::Identity; // 멤버 변수 사용
    return Bones[BoneIndex].CurrentLocalMatrix; // 멤버 변수 사용
}

bool FLoaderFBX::SetBoneWorldMatrix(uint32_t BoneIndex, const FMatrix& NewWorldMatrix)
{
    if (!Bones.IsValidIndex(BoneIndex)) return false; // 멤버 변수 사용

    FMatrix NewLocalMatrix;
    int32 ParentIdx = Bones[BoneIndex].ParentIndex; // 멤버 변수 사용

    if (ParentIdx != INDEX_NONE && Bones.IsValidIndex(ParentIdx)) { // 멤버 변수 사용
        const FMatrix& ParentCurrentWorld = Bones[ParentIdx].CurrentWorldTransform; // 멤버 변수 사용
        FMatrix ParentInverse = FMatrix::Inverse(ParentCurrentWorld);
        if (FMath::Abs(ParentInverse.Determinant()) < SMALL_NUMBER) ParentInverse = FMatrix::Identity;
        NewLocalMatrix = NewWorldMatrix * ParentInverse;
    }
    else {
        NewLocalMatrix = NewWorldMatrix;
    }
    Bones[BoneIndex].CurrentLocalMatrix = NewLocalMatrix; // 멤버 변수 사용
    // 중요: 호출 후 UpdateWorldTransforms() 필요
    return true;
}

FMatrix FLoaderFBX::GetBoneWorldMatrix(uint32_t BoneIndex) const
{
    if (!Bones.IsValidIndex(BoneIndex)) return FMatrix::Identity; // 멤버 변수 사용
    return Bones[BoneIndex].CurrentWorldTransform; // 멤버 변수 사용
}

uint32_t FLoaderFBX::GetBoneIndexByName(const FName& BoneName) const
{
    const int32* FoundIndex = BoneNameToIndexMap.Find(BoneName); // 멤버 맵 사용
    return FoundIndex ? *FoundIndex : INDEX_NONE;
}

// --- 재질 처리 (멤버 Materials 채움) ---
FBX::FFbxMaterialInfo FLoaderFBX::ProcessMaterial(FbxSurfaceMaterial* FbxMaterial)
{
    FBX::FFbxMaterialInfo MatInfo;
    if (!FbxMaterial) return MatInfo;

    MatInfo.MaterialName = FName(FbxMaterial->GetName()); // FString 생성자 사용 가정

    const char* DiffuseColorPropName = FbxSurfaceMaterial::sDiffuse;
    const char* BaseColorPropName = "BaseColor";
    const char* NormalMapPropName = FbxSurfaceMaterial::sNormalMap;
    const char* BumpMapPropName = FbxSurfaceMaterial::sBump;
    const char* MetallicPropName = "Metallic";
    const char* MetalnessPropName = "Metalness";
    const char* RoughnessPropName = "Roughness";
    const char* SpecularColorPropName = FbxSurfaceMaterial::sSpecular;
    const char* SpecularFactorPropName = FbxSurfaceMaterial::sSpecularFactor;
    const char* ShininessPropName = FbxSurfaceMaterial::sShininess;
    const char* EmissiveColorPropName = FbxSurfaceMaterial::sEmissive;
    const char* AmbientOcclusionPropName = "AmbientOcclusion";
    const char* OcclusionPropName = "Occlusion";
    const char* OpacityPropName = FbxSurfaceMaterial::sTransparencyFactor;
    const char* TransparencyPropName = FbxSurfaceMaterial::sTransparentColor;

    auto ExtractTexture = [&](const char* PropName, FWString& OutPath, bool& bOutHasTexture) {
        FbxProperty Property = FbxMaterial->FindProperty(PropName);
        bOutHasTexture = false; OutPath.clear();
        if (Property.IsValid()) {
            int TextureCount = Property.GetSrcObjectCount<FbxFileTexture>();
            if (TextureCount > 0) {
                FbxFileTexture* Texture = Property.GetSrcObject<FbxFileTexture>(0);
                if (Texture) {
                    OutPath = ProcessTexturePath(Texture); // 멤버 함수 호출
                    bOutHasTexture = !OutPath.empty();
                    if (bOutHasTexture) return;
                }
            }
        }
        };

    // Base Color
    FbxProperty BaseColorProp = FbxMaterial->FindProperty(BaseColorPropName);
    if (!BaseColorProp.IsValid()) BaseColorProp = FbxMaterial->FindProperty(DiffuseColorPropName);
    if (BaseColorProp.IsValid()) MatInfo.BaseColorFactor = ConvertFbxColorToLinear(BaseColorProp.Get<FbxDouble3>());
    ExtractTexture(BaseColorPropName, MatInfo.BaseColorTexturePath, MatInfo.bHasBaseColorTexture);
    if (!MatInfo.bHasBaseColorTexture) ExtractTexture(DiffuseColorPropName, MatInfo.BaseColorTexturePath, MatInfo.bHasBaseColorTexture);

    // Normal Map
    ExtractTexture(NormalMapPropName, MatInfo.NormalTexturePath, MatInfo.bHasNormalTexture);
    if (!MatInfo.bHasNormalTexture) ExtractTexture(BumpMapPropName, MatInfo.NormalTexturePath, MatInfo.bHasNormalTexture);

    // Emissive
    FbxProperty EmissiveProp = FbxMaterial->FindProperty(EmissiveColorPropName);
    if (EmissiveProp.IsValid()) MatInfo.EmissiveFactor = ConvertFbxColorToLinear(EmissiveProp.Get<FbxDouble3>());
    ExtractTexture(EmissiveColorPropName, MatInfo.EmissiveTexturePath, MatInfo.bHasEmissiveTexture);

    // Opacity
    FbxProperty OpacityProp = FbxMaterial->FindProperty(OpacityPropName);
    FbxProperty TransparentColorProp = FbxMaterial->FindProperty(TransparencyPropName);
    if (OpacityProp.IsValid()) {
        MatInfo.OpacityFactor = 1.0f - static_cast<float>(OpacityProp.Get<FbxDouble>());
    }
    else if (TransparentColorProp.IsValid()) {
        FbxDouble3 TransparentColor = TransparentColorProp.Get<FbxDouble3>();
        MatInfo.OpacityFactor = 1.0f - static_cast<float>((TransparentColor[0] + TransparentColor[1] + TransparentColor[2]) / 3.0);
    }
    else { MatInfo.OpacityFactor = 1.0f; }
    ExtractTexture("map_d", MatInfo.OpacityTexturePath, MatInfo.bHasOpacityTexture);

    // PBR / Traditional
    FbxProperty MetallicProp = FbxMaterial->FindProperty(MetallicPropName);
    if (!MetallicProp.IsValid()) MetallicProp = FbxMaterial->FindProperty(MetalnessPropName);
    FbxProperty RoughnessProp = FbxMaterial->FindProperty(RoughnessPropName);

    if (MetallicProp.IsValid() || RoughnessProp.IsValid()) { // PBR
        MatInfo.bUsePBRWorkflow = true;
        if (MetallicProp.IsValid()) MatInfo.MetallicFactor = static_cast<float>(MetallicProp.Get<FbxDouble>());
        if (RoughnessProp.IsValid()) MatInfo.RoughnessFactor = static_cast<float>(RoughnessProp.Get<FbxDouble>());
        ExtractTexture(MetallicPropName, MatInfo.MetallicTexturePath, MatInfo.bHasMetallicTexture);
        if (!MatInfo.bHasMetallicTexture) ExtractTexture(MetalnessPropName, MatInfo.MetallicTexturePath, MatInfo.bHasMetallicTexture);
        ExtractTexture(RoughnessPropName, MatInfo.RoughnessTexturePath, MatInfo.bHasRoughnessTexture);
        ExtractTexture(AmbientOcclusionPropName, MatInfo.AmbientOcclusionTexturePath, MatInfo.bHasAmbientOcclusionTexture);
        if (!MatInfo.bHasAmbientOcclusionTexture) ExtractTexture(OcclusionPropName, MatInfo.AmbientOcclusionTexturePath, MatInfo.bHasAmbientOcclusionTexture);
        MatInfo.SpecularFactor = FVector(0.04f, 0.04f, 0.04f); MatInfo.SpecularPower = 0.0f;
    }
    else { // Traditional
        MatInfo.bUsePBRWorkflow = false;
        FbxProperty SpecularColorProp = FbxMaterial->FindProperty(SpecularColorPropName);
        FbxProperty SpecularFactorProp = FbxMaterial->FindProperty(SpecularFactorPropName);
        if (SpecularColorProp.IsValid()) {
            MatInfo.SpecularFactor = ConvertFbxColorToLinear(SpecularColorProp.Get<FbxDouble3>()).ToVector3();
            if (SpecularFactorProp.IsValid()) MatInfo.SpecularFactor *= static_cast<float>(SpecularFactorProp.Get<FbxDouble>());
        }
        else if (SpecularFactorProp.IsValid()) {
            float factor = static_cast<float>(SpecularFactorProp.Get<FbxDouble>());
            MatInfo.SpecularFactor = FVector(factor, factor, factor);
        }
        ExtractTexture(SpecularColorPropName, MatInfo.SpecularTexturePath, MatInfo.bHasSpecularTexture);
        FbxProperty ShininessProp = FbxMaterial->FindProperty(ShininessPropName);
        if (ShininessProp.IsValid()) MatInfo.SpecularPower = static_cast<float>(ShininessProp.Get<FbxDouble>());
        MatInfo.MetallicFactor = 0.0f;
        MatInfo.RoughnessFactor = FMath::Clamp(FMath::Sqrt(2.0f / (MatInfo.SpecularPower + 2.0f)), 0.0f, 1.0f);
    }

    MatInfo.bIsTransparent = (MatInfo.OpacityFactor < 1.0f - KINDA_SMALL_NUMBER) || MatInfo.bHasOpacityTexture;
    return MatInfo;
}

// --- FBX Element 데이터 추출 템플릿 함수 ---
template<typename TElementType, typename TDataType>
bool FLoaderFBX::GetElementData(FbxMesh* Mesh, TElementType* Element, int32 ControlPointCount, int32 PolygonVertexCount,
    const TArray<int32>& PolygonVertexIndices,
    TArray<TDataType>& OutControlPointData,
    TArray<TDataType>& OutPolygonVertexData)
{
    if (!Element) return false;
    FbxGeometryElement::EMappingMode MappingMode = Element->GetMappingMode();
    FbxGeometryElement::EReferenceMode ReferenceMode = Element->GetReferenceMode();
    const auto& DirectArray = Element->GetDirectArray();
    const auto& IndexArray = Element->GetIndexArray();
    const int32 DirectDataCount = DirectArray.GetCount();
    const int32 IndexArrayCount = IndexArray.GetCount();
    TDataType DefaultValue = {};
    if constexpr (std::is_same_v<TDataType, FVector>) DefaultValue = FVector(0, 0, 1);
    else if constexpr (std::is_same_v<TDataType, FVector2D>) DefaultValue = FVector2D(0, 0);

    auto GetDataValue = [&](int DataIndex) -> TDataType {
        if (DataIndex >= 0 && DataIndex < DirectDataCount) {
            if constexpr (std::is_same_v<TDataType, FVector>) return ConvertFbxNormal(DirectArray.GetAt(DataIndex));
            else if constexpr (std::is_same_v<TDataType, FVector2D>) return ConvertFbxUV(DirectArray.GetAt(DataIndex));
        } return DefaultValue;
        };

    if (MappingMode == FbxGeometryElement::eByControlPoint) {
        OutControlPointData.SetNum(ControlPointCount);
        if (ReferenceMode == FbxGeometryElement::eDirect) {
            for (int32 i = 0; i < ControlPointCount; ++i) OutControlPointData[i] = GetDataValue(i); return true;
        }
        else if (ReferenceMode == FbxGeometryElement::eIndexToDirect) {
            for (int32 i = 0; i < ControlPointCount; ++i) {
                if (i < IndexArrayCount) OutControlPointData[i] = GetDataValue(IndexArray.GetAt(i));
                else OutControlPointData[i] = DefaultValue;
            } return true;
        }
    }
    else if (MappingMode == FbxGeometryElement::eByPolygonVertex) {
        OutPolygonVertexData.SetNum(PolygonVertexCount);
        if (ReferenceMode == FbxGeometryElement::eDirect) {
            for (int32 i = 0; i < PolygonVertexCount; ++i) OutPolygonVertexData[i] = GetDataValue(i); return true;
        }
        else if (ReferenceMode == FbxGeometryElement::eIndexToDirect) {
            for (int32 i = 0; i < PolygonVertexCount; ++i) {
                if (i < IndexArrayCount) OutPolygonVertexData[i] = GetDataValue(IndexArray.GetAt(i));
                else OutPolygonVertexData[i] = DefaultValue;
            } return true;
        }
    }
    return false;
}

// --- 텍스처 경로 처리 헬퍼 (멤버 LoadedFBXFileDirectory 사용) ---
FWString FLoaderFBX::ProcessTexturePath(FbxFileTexture* Texture)
{
    if (!Texture) return FWString();

    // FString 생성자 사용 가정 (char* -> FString)
    FString RelativePath = FString(Texture->GetRelativeFileName());
    FString AbsolutePath = FString(Texture->GetFileName());
    FWString FinalPath;
    std::error_code ec;

    // 1. 상대 경로 시도 (멤버 LoadedFBXFileDirectory 사용)
    if (!RelativePath.IsEmpty() && !LoadedFBXFileDirectory.empty()) // 멤버 변수 사용
    {
        // FString을 std::filesystem::path로 변환 (FString이 wchar_t* 제공 가정)
        std::filesystem::path BaseDir(LoadedFBXFileDirectory); // 멤버 변수 사용
        std::filesystem::path RelPath(*RelativePath);
        std::filesystem::path CombinedPath = BaseDir / RelPath;
        CombinedPath = std::filesystem::absolute(CombinedPath, ec);
        if (!ec) {
            CombinedPath.make_preferred();
            if (std::filesystem::exists(CombinedPath, ec) && !ec) {
                FinalPath = CombinedPath.wstring().c_str(); // wstring -> FString 변환 가정
            }
            else { ec.clear(); }
        }
        else { ec.clear(); }
    }

    // 2. 절대 경로 시도
    if (FinalPath.empty() && !AbsolutePath.IsEmpty())
    {
        std::filesystem::path AbsPath(*AbsolutePath);
        AbsPath.make_preferred();
        if (std::filesystem::exists(AbsPath, ec) && !ec) {
            FinalPath = AbsPath.wstring().c_str(); // wstring -> FString 변환 가정
        }
        else { ec.clear(); }
    }

    // 3. 파일 이름만 반환 시도
    if (FinalPath.empty()) {
        FString PathToExtractFrom = AbsolutePath.IsEmpty() ? RelativePath : AbsolutePath;
        if (!PathToExtractFrom.IsEmpty()) {
            std::filesystem::path TempPath(*PathToExtractFrom);
            if (TempPath.has_filename()) {
                FinalPath = TempPath.filename().wstring().c_str(); // wstring -> FString 변환 가정
            }
        }
    }

    // 필요시 경로 정규화 (예: 역슬래시 -> 슬래시)
    // FinalPath.ReplaceInline(TEXT("\\"), TEXT("/")); // FString 기능 사용 가정

    return FinalPath;
}
// --- 좌표계 변환 헬퍼 구현 ---
FVector FLoaderFBX::ConvertFbxPosition(const FbxVector4& Vector)
{
    return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
}

FVector FLoaderFBX::ConvertFbxNormal(const FbxVector4& Vector)
{
    return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
}
FVector2D FLoaderFBX::ConvertFbxUV(const FbxVector2& Vector)
{
    return FVector2D(static_cast<float>(Vector[0]), 1.0f - static_cast<float>(Vector[1]));
}
