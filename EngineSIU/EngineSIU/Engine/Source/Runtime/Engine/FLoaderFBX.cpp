#include "FLoaderFBX.h"
#include "Define.h" // FPaths 등 사용 시
#include "Math/Vector.h" // FMath 사용 시
#include <functional> // std::function
#include <numeric>    // std::accumulate 등 (필요시)
#include <algorithm>  // std::sort, std::find 등
#include <fbxsdk/utils/fbxrootnodeutility.h>
// 임시 구조체: 제어점별 스키닝 데이터 누적용
struct FControlPointSkinningData
{
    struct FBoneInfluence
    {
        int32 BoneIndex = INDEX_NONE;
        float Weight = 0.0f;

        // 정렬을 위한 비교 연산자
        bool operator>(const FBoneInfluence& Other) const { return Weight > Other.Weight; }
    };
    TArray<FBoneInfluence> Influences;

    void NormalizeWeights(int32 MaxInfluences)
    {
        if (Influences.IsEmpty()) return;

        // 1. 가중치 합계 계산
        float TotalWeight = 0.0f;
        for (const auto& Influence : Influences)
        {
            TotalWeight += Influence.Weight;
        }

        // 2. 정규화 (합계가 0에 가까우면 균등 분배 시도)
        if (TotalWeight > KINDA_SMALL_NUMBER)
        {
            for (auto& Influence : Influences)
            {
                Influence.Weight /= TotalWeight;
            }
        }
        else if (!Influences.IsEmpty()) // 합계가 0인데 영향이 있다면
        {
            float EqualWeight = 1.0f / Influences.Num();
            for (auto& Influence : Influences)
            {
                Influence.Weight = EqualWeight;
            }
        }

        // 3. 가중치 높은 순으로 정렬
        Influences.Sort(std::greater<FBoneInfluence>()); // 내림차순 정렬

        // 4. 최대 영향 수 제한
        if (Influences.Num() > MaxInfluences)
        {
            Influences.SetNum(MaxInfluences);

            // 제한 후 다시 정규화
            TotalWeight = 0.0f;
            for (const auto& Influence : Influences)
            {
                TotalWeight += Influence.Weight;
            }

            if (TotalWeight > KINDA_SMALL_NUMBER)
            {
                for (auto& Influence : Influences)
                {
                    Influence.Weight /= TotalWeight;
                }
            }
            else if (!Influences.IsEmpty()) // 이론상 발생하기 어려움
            {
                Influences[0].Weight = 1.0f;
                for (int32 i = 1; i < Influences.Num(); ++i) Influences[i].Weight = 0.0f;
            }
        }

        // 5. 최종 합계가 1이 되도록 조정 (부동소수점 오차 보정)
        if (!Influences.IsEmpty())
        {
            float CurrentSum = 0.0f;
            for (int32 i = 0; i < Influences.Num() - 1; ++i)
            {
                CurrentSum += Influences[i].Weight;
            }
            int32 LastIndex = Influences.Num() - 1;
            if (LastIndex >= 0)
            {
                Influences[LastIndex].Weight = FMath::Max(0.0f, 1.0f - CurrentSum);
            }
        }

        // 6. (선택) 가중치가 0인 항목 제거 또는 BoneIndices/Weights 배열 채우기 준비
        // FMeshVertex 채울 때 0인 항목은 자동으로 처리됨
    }
};


// --- 생성자 / 소멸자 ---


FLoaderFBX::~FLoaderFBX()
{
    ReleaseBuffers();
    ShutdownFBXSDK();
}

// --- 로딩 및 초기화 ---

bool FLoaderFBX::InitializeFBXSDK()
{
    if (SdkManager) return true; // 이미 초기화됨

    SdkManager = FbxManager::Create();
    if (!SdkManager)
    {
        // UE_LOG(LogLevel::Error, TEXT("Failed to create FbxManager."));
        return false;
    }

    FbxIOSettings* IOS = FbxIOSettings::Create(SdkManager, IOSROOT);
    SdkManager->SetIOSettings(IOS);

    Scene = FbxScene::Create(SdkManager, "ImportScene");
    if (!Scene)
    {
        // UE_LOG(LogLevel::Error, TEXT("Failed to create FbxScene."));
        ShutdownFBXSDK(); // 생성된 Manager 정리
        return false;
    }
    return true;
}

void FLoaderFBX::ShutdownFBXSDK()
{
    if (Scene)
    {
        Scene->Destroy();
        Scene = nullptr;
    }
    if (SdkManager)
    {
        SdkManager->Destroy();
        SdkManager = nullptr;
    }
}

void FLoaderFBX::ReleaseBuffers()
{
    // Raw 포인터 사용 시:
    if (DynamicVertexBuffer) { DynamicVertexBuffer->Release(); DynamicVertexBuffer = nullptr; }
    if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = nullptr; }
}

bool FLoaderFBX::LoadFBXFile(const FString& Filepath, ID3D11Device* Device)
{
    if (!Device) return false;

    // 1. 기존 리소스 정리
    ReleaseBuffers();
    BindPoseVertices.Empty();
    FinalIndices.Empty();
    Bones.Empty();
    BoneNameToIndexMap.Empty();
    ShutdownFBXSDK();

    // 2. SDK 초기화
    if (!InitializeFBXSDK()) return false;

    // 3. Importer 생성
    FbxImporter* Importer = FbxImporter::Create(SdkManager, "");
    if (!Importer) { ShutdownFBXSDK(); return false; }

    // 4. 파일 경로 → UTF-8
    std::string FilepathUTF8 = (*Filepath);
    bool bInit = Importer->Initialize(FilepathUTF8.c_str(), -1, SdkManager->GetIOSettings());
    if (!bInit)
    {
        Importer->Destroy();
        ShutdownFBXSDK();
        return false;
    }

    // 5. 씬 임포트
    if (!Importer->Import(Scene))
    {
        Importer->Destroy();
        ShutdownFBXSDK();
        return false;
    }
    Importer->Destroy();

    // 6. 축(axis) 변환
    FbxAxisSystem targetAxis(FbxAxisSystem::eZAxis,
        FbxAxisSystem::eParityOdd,
        FbxAxisSystem::eLeftHanded);
    FbxAxisSystem sourceAxis = Scene->GetGlobalSettings().GetAxisSystem();
    if (sourceAxis != targetAxis)
    {
        targetAxis.ConvertScene(Scene);
    }

    // 7. 단위(unit) 변환 (예: 센티미터 → 미터)
    FbxSystemUnit::cm.ConvertScene(Scene);

    FbxRootNodeUtility::RemoveAllFbxRoots(Scene);

    // 9. 삼각화
    FbxGeometryConverter GeometryConverter(SdkManager);
    GeometryConverter.Triangulate(Scene, true);

    // 10. 씬 처리
    if (!ProcessScene())
    {
        ShutdownFBXSDK();
        return false;
    }

    // 11. DirectX 버퍼 생성
    if (!CreateBuffers(Device))
    {
        ReleaseBuffers();
        ShutdownFBXSDK();
        return false;
    }

    return true;
}

bool FLoaderFBX::ProcessScene()
{
    FbxNode* RootNode = Scene->GetRootNode();
    if (!RootNode)
    {
        // UE_LOG(LogLevel::Error, TEXT("Failed to get root node from the scene."));
        return false;
    }

    bool bMeshFoundAndProcessed = false;
    // 1. 메쉬 노드 찾아서 처리 (첫 번째 메쉬만 처리하는 예시)
    for (int i = 0; i < RootNode->GetChildCount(); ++i)
    {
        FbxNode* ChildNode = RootNode->GetChild(i);
        FbxMesh* Mesh = ChildNode->GetMesh();
        if (Mesh)
        {
            // UE_LOG(LogLevel::Display, TEXT("Processing Mesh Node: %s"), UTF8_TO_TCHAR(ChildNode->GetName()));
            if (ProcessMesh(Mesh))
            {
                bMeshFoundAndProcessed = true;
                // 첫 번째 메쉬만 처리하고 중단 (필요에 따라 여러 메쉬 처리 로직 추가)
                break;
            }
            else
            {
                // UE_LOG(LogLevel::Error, TEXT("Failed to process mesh: %s"), UTF8_TO_TCHAR(ChildNode->GetName()));
                return false; // 메쉬 처리 실패 시 중단
            }
        }
        // 재귀적으로 모든 노드를 탐색하여 메쉬를 찾을 수도 있음
        // ProcessNodeRecursive(ChildNode); // <- 이 함수 내에서 메쉬 처리 호출
    }

    if (!bMeshFoundAndProcessed)
    {
        // UE_LOG(LogLevel::Warning, TEXT("No mesh found or processed in the scene. Trying recursive search..."));
        // 재귀 탐색 로직 추가 가능
        ProcessNodeRecursive(RootNode); // 재귀 탐색 시도
        if (BindPoseVertices.IsEmpty()) // 재귀 탐색 후에도 메쉬 데이터가 없으면
        {
            // UE_LOG(LogLevel::Warning, TEXT("No mesh data loaded after recursive search."));
            return false; // 메쉬가 없으면 로드 실패 처리
        }
        bMeshFoundAndProcessed = true; // 재귀에서 찾았다고 가정
    }


    // 2. 스켈레톤 계층 구조 처리 (메쉬 처리 후 Bones 정보가 채워진 상태에서 수행)
    if (bMeshFoundAndProcessed && !Bones.IsEmpty())
    {
        if (!ProcessSkeletonHierarchy(RootNode))
        {
            // UE_LOG(LogLevel::Warning, TEXT("Failed to fully process skeleton hierarchy, but continuing."));
            // 계층 구조 처리 실패는 치명적이지 않을 수 있음 (ParentIndex가 설정되지 않음)
        }
    }
    else if (bMeshFoundAndProcessed && Bones.IsEmpty())
    {
        // UE_LOG(LogLevel::Display, TEXT("Mesh processed, but no skeleton/skinning data found."));
    }

    return bMeshFoundAndProcessed; // 메쉬가 성공적으로 처리되었는지 여부 반환
}

// 재귀적으로 노드를 탐색하며 메쉬를 찾는 함수 (ProcessScene에서 호출 가능)
void FLoaderFBX::ProcessNodeRecursive(FbxNode* Node)
{
    if (!Node) return;

    // 메쉬 처리 (아직 처리되지 않았다면)
    if (BindPoseVertices.IsEmpty()) // 메쉬 데이터가 아직 없다면
    {
        FbxMesh* Mesh = Node->GetMesh();
        if (Mesh)
        {
            // UE_LOG(LogLevel::Display, TEXT("Processing Mesh Node (Recursive): %s"), UTF8_TO_TCHAR(Node->GetName()));
            if (!ProcessMesh(Mesh))
            {
                // UE_LOG(LogLevel::Warning, TEXT("Failed to process mesh during recursive search: %s"), UTF8_TO_TCHAR(Node->GetName()));
            }
            // 메쉬를 찾으면 더 이상 재귀할 필요 없을 수 있음 (구현에 따라 다름)
            // return; // 첫 메쉬만 찾으면 종료
        }
    }

    // 자식 노드 재귀 호출
    for (int i = 0; i < Node->GetChildCount(); ++i)
    {
        ProcessNodeRecursive(Node->GetChild(i));
        // if (!BindPoseVertices.IsEmpty()) return; // 메쉬를 찾으면 종료
    }
}


bool FLoaderFBX::ProcessMesh(FbxMesh* Mesh)
{
    if (!Mesh) return false;

    const int32 ControlPointCount = Mesh->GetControlPointsCount();
    if (ControlPointCount <= 0)
    {
        // UE_LOG(LogLevel::Warning, TEXT("Mesh '%s' has no control points."), UTF8_TO_TCHAR(Mesh->GetName()));
        return false;
    }

    // 1. 제어점 위치 (Control Points)
    TArray<FVector> ControlPointPositions;
    ControlPointPositions.Reserve(ControlPointCount);
    FbxVector4* FbxControlPoints = Mesh->GetControlPoints();
    for (int32 i = 0; i < ControlPointCount; ++i)
    {
        ControlPointPositions.Add(ConvertFbxPosition(FbxControlPoints[i]));
    }

    // 2. 폴리곤 정점 인덱스 (Polygon Vertex Indices)
    const int32 PolygonVertexCount = Mesh->GetPolygonVertexCount();
    if (PolygonVertexCount <= 0 || PolygonVertexCount % 3 != 0) // 삼각화 가정
    {
        // UE_LOG(LogLevel::Warning, TEXT("Mesh '%s' has no polygon vertices or is not triangulated (VertexCount: %d)."), UTF8_TO_TCHAR(Mesh->GetName()), PolygonVertexCount);
        return false;
    }
    TArray<int32> PolygonIndices;
    PolygonIndices.Reserve(PolygonVertexCount);
    int* FbxPolygonVertices = Mesh->GetPolygonVertices();
    for (int32 i = 0; i < PolygonVertexCount; i += 3) // 3개씩 (삼각형 단위로) 증가
    {
        if (i + 2 < PolygonVertexCount) // 배열 범위 확인
        {
            PolygonIndices.Add(FbxPolygonVertices[i]);     // 정점 0
            PolygonIndices.Add(FbxPolygonVertices[i + 2]); // 정점 2
            PolygonIndices.Add(FbxPolygonVertices[i + 1]); // 정점 1
        }
        else { break; }
    }

    // 3. 노멀 데이터
    TArray<FVector> ControlPointNormals;
    TArray<FVector> PolygonVertexNormals;
    FbxGeometryElementNormal* NormalElement = Mesh->GetElementNormal(0); // 첫 번째 노멀 세트 사용
    bool bHasNormals = false;
    if (NormalElement)
    {
        bHasNormals = GetElementData<FbxGeometryElementNormal, FVector>(
            Mesh, NormalElement, ControlPointCount, PolygonVertexCount, PolygonIndices,
            ControlPointNormals, PolygonVertexNormals
        );
        if (!bHasNormals)
        {
            // UE_LOG(LogLevel::Warning, TEXT("Failed to extract normal data for mesh '%s'. Using default normals."), UTF8_TO_TCHAR(Mesh->GetName()));
        }
    }
    else
    {
        // UE_LOG(LogLevel::Warning, TEXT("Mesh '%s' does not contain normal data. Using default normals."), UTF8_TO_TCHAR(Mesh->GetName()));
    }

    // 4. UV 데이터 (텍스처 좌표)
    TArray<FVector2D> ControlPointUVs;
    TArray<FVector2D> PolygonVertexUVs;
    FbxGeometryElementUV* UVElement = Mesh->GetElementUV(0); // 첫 번째 UV 세트 사용 (일반적으로 Diffuse)
    bool bHasUVs = false;
    if (UVElement)
    {
        // UV 세트 이름 확인 (선택 사항)
        // FbxString UVSetName = UVElement->GetName();
        // UE_LOG(LogLevel::Display, TEXT("Using UV Set: %s"), UTF8_TO_TCHAR(UVSetName.Buffer()));

        bHasUVs = GetElementData<FbxGeometryElementUV, FVector2D>(
            Mesh, UVElement, ControlPointCount, PolygonVertexCount, PolygonIndices,
            ControlPointUVs, PolygonVertexUVs
        );
        if (!bHasUVs)
        {
            // UE_LOG(LogLevel::Warning, TEXT("Failed to extract UV data for mesh '%s'. Using default UVs."), UTF8_TO_TCHAR(Mesh->GetName()));
        }
    }
    else
    {
        // UE_LOG(LogLevel::Warning, TEXT("Mesh '%s' does not contain UV data. Using default UVs."), UTF8_TO_TCHAR(Mesh->GetName()));
    }

    // 5. 스키닝 데이터 추출
    TArray<FControlPointSkinningData> CpSkinData;
    CpSkinData.SetNum(ControlPointCount); // 제어점 개수만큼 초기화
    ExtractSkinningData(Mesh, CpSkinData);

    // 6. 제어점별 가중치 정규화 및 상위 N개 선택
    for (int32 i = 0; i < ControlPointCount; ++i)
    {
        if (CpSkinData.IsValidIndex(i))
        {
            CpSkinData[i].NormalizeWeights(MAX_BONE_INFLUENCES);
        }
    }

    // 7. 최종 정점 및 인덱스 데이터 생성 (중복 제거 포함)
    FinalizeVertexData(
        ControlPointPositions, PolygonIndices,
        ControlPointNormals, PolygonVertexNormals, NormalElement,
        ControlPointUVs, PolygonVertexUVs, UVElement,
        CpSkinData
    );

    return !BindPoseVertices.IsEmpty() && !FinalIndices.IsEmpty();
}
void FLoaderFBX::ExtractSkinningData(FbxMesh* Mesh, TArray<FControlPointSkinningData>& OutCpSkinData)
{
    const int32 ControlPointCount = Mesh->GetControlPointsCount();
    if (ControlPointCount <= 0) return;

    const int32 DeformerCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
    // UE_LOG(LogLevel::Display, TEXT("Found %d skin deformers."), DeformerCount);

    for (int32 DeformerIndex = 0; DeformerIndex < DeformerCount; ++DeformerIndex)
    {
        FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(DeformerIndex, FbxDeformer::eSkin));
        if (!Skin) continue;

        const int32 ClusterCount = Skin->GetClusterCount();

        for (int32 ClusterIndex = 0; ClusterIndex < ClusterCount; ++ClusterIndex)
        {
            FbxCluster* Cluster = Skin->GetCluster(ClusterIndex);
            FbxNode* BoneNode = Cluster->GetLink(); // 이 클러스터(뼈)에 연결된 노드
            if (!BoneNode) continue;

            FName BoneName = FName(BoneNode->GetName());
            int32 CurrentBoneIndex = INDEX_NONE;

            // 뼈 목록(Bones)에 이미 있는지 확인, 없으면 추가
            int32* FoundIndexPtr = BoneNameToIndexMap.Find(BoneName);
            if (FoundIndexPtr == nullptr) // 새 뼈 발견
            {
                CurrentBoneIndex = Bones.Emplace();
                if (Bones.IsValidIndex(CurrentBoneIndex))
                {
                    FBoneInfo& NewBone = Bones[CurrentBoneIndex];
                    NewBone.Name = BoneName;
                    NewBone.ParentIndex = INDEX_NONE;

                    FbxAMatrix BindPoseMatrix_Unused, TransformLinkMatrix; // BindPoseMatrix는 여전히 사용 안 함
                    Cluster->GetTransformMatrix(BindPoseMatrix_Unused);
                    Cluster->GetTransformLinkMatrix(TransformLinkMatrix);

                    FbxAMatrix InvBindPoseFbx = TransformLinkMatrix.Inverse();

                    NewBone.InverseBindPoseMatrix = FMatrix::ConvertFbxAMatrixToFMatrix(InvBindPoseFbx);
                    NewBone.BindPoseMatrix = FMatrix::ConvertFbxAMatrixToFMatrix(TransformLinkMatrix);

                    BoneNameToIndexMap.Add(BoneName, CurrentBoneIndex);
                }
                else { continue; }
            }
            else // 이미 존재하는 뼈
            {
                CurrentBoneIndex = *FoundIndexPtr;
            }

            // 이 뼈(클러스터)가 영향을 미치는 제어점과 가중치 가져오기
            int* ControlPointIndices = Cluster->GetControlPointIndices();
            double* ControlPointWeights = Cluster->GetControlPointWeights();
            const int32 InfluenceCount = Cluster->GetControlPointIndicesCount();

            for (int32 InfluenceIndex = 0; InfluenceIndex < InfluenceCount; ++InfluenceIndex)
            {
                int32 CPIndex = ControlPointIndices[InfluenceIndex];
                float Weight = static_cast<float>(ControlPointWeights[InfluenceIndex]);

                // 유효한 제어점 인덱스이고 가중치가 의미있는 값일 경우 추가
                if (OutCpSkinData.IsValidIndex(CPIndex) && Weight > KINDA_SMALL_NUMBER)
                {
                    // CurrentBoneIndex가 유효하게 설정되었는지 확인 (새 뼈 추가 실패 시 INDEX_NONE일 수 있음)
                    if (CurrentBoneIndex != INDEX_NONE)
                    {
                        OutCpSkinData[CPIndex].Influences.Add({ CurrentBoneIndex, Weight });
                    }
                }
            }
        } // End Cluster Loop
    } // End Deformer Loop
}

void FLoaderFBX::FinalizeVertexData(
    const TArray<FVector>& ControlPointPositions,
    const TArray<int32>& PolygonIndices,
    const TArray<FVector>& ControlPointNormals,
    const TArray<FVector>& PolygonVertexNormals,
    const FbxGeometryElementNormal* NormalElement,
    const TArray<FVector2D>& ControlPointUVs,
    const TArray<FVector2D>& PolygonVertexUVs,
    const FbxGeometryElementUV* UVElement,
    const TArray<struct FControlPointSkinningData>& CpSkinData)
{
    BindPoseVertices.Empty();
    FinalIndices.Empty();
    TMap<FMeshVertex, uint32> UniqueVertices; // 중복 정점 제거용 맵

    const int32 PolygonVertexCount = PolygonIndices.Num();
    FinalIndices.Reserve(PolygonVertexCount); // 인덱스 버퍼 크기 예약

    for (int32 PolyVertIndex = 0; PolyVertIndex < PolygonVertexCount; ++PolyVertIndex)
    {
        const int32 ControlPointIndex = PolygonIndices[PolyVertIndex];
        if (!ControlPointPositions.IsValidIndex(ControlPointIndex))
        {
            // UE_LOG(LogLevel::Error, TEXT("Invalid ControlPointIndex %d encountered at PolygonVertexIndex %d."), ControlPointIndex, PolyVertIndex);
            continue; // 이 정점 건너뛰기
        }

        FMeshVertex CurrentVertex = {}; // 현재 처리 중인 최종 정점 데이터

        // 1. 위치 설정
        CurrentVertex.Position = ControlPointPositions[ControlPointIndex];

        // 2. 노멀 설정
        if (NormalElement)
        {
            if (NormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
            {
                if (ControlPointNormals.IsValidIndex(ControlPointIndex))
                    CurrentVertex.Normal = ControlPointNormals[ControlPointIndex];
                else CurrentVertex.Normal = FVector(0.f, 0.f, 1.f); // 기본값
            }
            else // eByPolygonVertex
            {
                if (PolygonVertexNormals.IsValidIndex(PolyVertIndex))
                    CurrentVertex.Normal = PolygonVertexNormals[PolyVertIndex];
                else CurrentVertex.Normal = FVector(0.f, 0.f, 1.f); // 기본값
            }
        }
        else
        {
            CurrentVertex.Normal = FVector(0.f, 0.f, 1.f); // 노멀 데이터 없음
        }
        CurrentVertex.Normal.Normalize(); // 정규화

        // 3. UV 설정
        if (UVElement)
        {
            if (UVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
            {
                if (ControlPointUVs.IsValidIndex(ControlPointIndex))
                    CurrentVertex.TexCoord = ControlPointUVs[ControlPointIndex];
                else CurrentVertex.TexCoord = FVector2D(0.f, 0.f); // 기본값
            }
            else // eByPolygonVertex
            {
                if (PolygonVertexUVs.IsValidIndex(PolyVertIndex))
                    CurrentVertex.TexCoord = PolygonVertexUVs[PolyVertIndex];
                else CurrentVertex.TexCoord = FVector2D(0.f, 0.f); // 기본값
            }
        }
        else
        {
            CurrentVertex.TexCoord = FVector2D(0.f, 0.f); // UV 데이터 없음
        }

        // 4. 스키닝 데이터 설정 (Bone Indices & Weights)
        if (CpSkinData.IsValidIndex(ControlPointIndex))
        {
            const auto& Influences = CpSkinData[ControlPointIndex].Influences;
            for (int32 i = 0; i < Influences.Num() && i < MAX_BONE_INFLUENCES; ++i)
            {
                CurrentVertex.BoneIndices[i] = Influences[i].BoneIndex;
                CurrentVertex.BoneWeights[i] = Influences[i].Weight;
            }
            // 나머지는 0으로 초기화되어 있음
        }
        // else: 스키닝 데이터 없는 경우 (정적 메쉬 등), 기본값(0) 유지

        // 5. 중복 정점 확인 및 최종 데이터 추가
        uint32* FoundIndexPtr = UniqueVertices.Find(CurrentVertex);
        if (FoundIndexPtr != nullptr)
        {
            // 이미 존재하는 정점 -> 인덱스만 추가
            FinalIndices.Add(*FoundIndexPtr);
        }
        else
        {
            // 새로운 고유 정점 -> 정점 배열에 추가하고, 새 인덱스를 맵과 인덱스 배열에 추가
            uint32 NewIndex = static_cast<uint32>(BindPoseVertices.Add(CurrentVertex));
            UniqueVertices.Add(CurrentVertex, NewIndex);
            FinalIndices.Add(NewIndex);
        }
    } // End Polygon Vertex Loop

    // UE_LOG(LogLevel::Display, TEXT("Finalized Vertex Data - Vertices: %d, Indices: %d"), BindPoseVertices.Num(), FinalIndices.Num());
}


// --- 스켈레톤 계층 구조 처리 ---

bool FLoaderFBX::ProcessSkeletonHierarchy(FbxNode* RootNode)
{
    if (!RootNode || Bones.IsEmpty()) // 뼈 정보가 없으면 처리할 필요 없음
    {
        return Bones.IsEmpty(); // 뼈가 없으면 성공으로 간주
    }
    // UE_LOG(LogLevel::Display, TEXT("Processing skeleton hierarchy..."));

    // 루트부터 시작하여 재귀적으로 뼈 노드의 부모-자식 관계 설정
    for (int i = 0; i < RootNode->GetChildCount(); ++i)
    {
        ProcessSkeletonNodeRecursive(RootNode->GetChild(i), INDEX_NONE); // 루트 노드의 자식부터 시작, 부모는 없음(INDEX_NONE)
    }

    // 루트 뼈 확인 (ParentIndex가 여전히 INDEX_NONE인 뼈)
    int rootBoneCount = 0;
    for (const auto& bone : Bones) {
        if (bone.ParentIndex == INDEX_NONE) {
            rootBoneCount++;
        }
    }
    // UE_LOG(LogLevel::Display, TEXT("Skeleton hierarchy processing complete. Found %d root bone(s)."), rootBoneCount);

    // 모든 뼈가 부모를 찾았는지 확인 (선택적 검증)
    bool bAllBonesHaveParentOrAreRoot = true;
    for (int32 i = 0; i < Bones.Num(); ++i)
    {

    }

    return bAllBonesHaveParentOrAreRoot;
}

void FLoaderFBX::ProcessSkeletonNodeRecursive(FbxNode* Node, int32 CurrentParentBoneIndex)
{
    if (!Node) return;

    FName NodeName = FName(Node->GetName());
    int32 ThisNodeBoneIndex = INDEX_NONE;

    // 현재 노드가 스켈레톤 노드이고, 우리 뼈 목록에 있는지 확인
    FbxNodeAttribute* NodeAttribute = Node->GetNodeAttribute();
    if (NodeAttribute && NodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        int32* FoundIndexPtr = BoneNameToIndexMap.Find(NodeName);
        if (FoundIndexPtr != nullptr) // 우리 뼈 목록에 있는 뼈 노드임
        {
            ThisNodeBoneIndex = *FoundIndexPtr;
            if (Bones.IsValidIndex(ThisNodeBoneIndex))
            {
                // 부모 인덱스가 아직 설정되지 않았다면 설정
                if (Bones[ThisNodeBoneIndex].ParentIndex == INDEX_NONE)
                {
                    Bones[ThisNodeBoneIndex].ParentIndex = CurrentParentBoneIndex;
                }
                // 이미 설정된 부모와 다른 경우 경고 (FBX 구조가 이상할 수 있음)
                else if (Bones[ThisNodeBoneIndex].ParentIndex != CurrentParentBoneIndex)
                {
                }
            }
            else
            {
                ThisNodeBoneIndex = INDEX_NONE; // 유효하지 않은 인덱스 처리
            }
        }
    }

    for (int32 i = 0; i < Node->GetChildCount(); ++i)
    {
        int32 ParentIndexForChild = (ThisNodeBoneIndex != INDEX_NONE) ? ThisNodeBoneIndex : CurrentParentBoneIndex;
        ProcessSkeletonNodeRecursive(Node->GetChild(i), ParentIndexForChild);
    }
}


// --- DirectX 버퍼 생성 ---

bool FLoaderFBX::CreateBuffers(ID3D11Device* Device)
{
    if (!Device || BindPoseVertices.IsEmpty() || FinalIndices.IsEmpty())
    {
        return false;
    }

    ReleaseBuffers(); // 기존 버퍼 해제

    HRESULT hr;

    D3D11_BUFFER_DESC DynVertexBufferDesc = {};
    DynVertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // CPU 쓰기 가능, GPU 읽기 전용
    DynVertexBufferDesc.ByteWidth = sizeof(FMeshVertex) * BindPoseVertices.Num();
    DynVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    DynVertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU에서 쓰기 가능해야 함
    DynVertexBufferDesc.MiscFlags = 0;
    DynVertexBufferDesc.StructureByteStride = 0;

    // 초기 데이터 설정 (바인드 포즈 데이터로 초기화)
    D3D11_SUBRESOURCE_DATA VertexInitData = {};
    VertexInitData.pSysMem = BindPoseVertices.GetData(); // TArray의 데이터 포인터

    hr = Device->CreateBuffer(&DynVertexBufferDesc, &VertexInitData, &DynamicVertexBuffer);

    if (FAILED(hr))
    {
        return false;
    }

    // 2. 인덱스 버퍼 (Index Buffer) - 일반적으로 변경되지 않으므로 Default 사용
    D3D11_BUFFER_DESC IndexBufferDesc = {};
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT; // GPU 읽기 전용 (가장 성능 좋음)
    IndexBufferDesc.ByteWidth = sizeof(uint32) * FinalIndices.Num();
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0; // CPU 접근 불필요
    IndexBufferDesc.MiscFlags = 0;
    IndexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA IndexInitData = {};
    IndexInitData.pSysMem = FinalIndices.GetData();

    hr = Device->CreateBuffer(&IndexBufferDesc, &IndexInitData, &IndexBuffer);

    if (FAILED(hr))
    {
        ReleaseBuffers(); // 실패 시 생성된 정점 버퍼도 해제
        return false;
    }

    // UE_LOG(LogLevel::Display, TEXT("DirectX buffers created successfully."));
    return true;
}
bool FLoaderFBX::UpdateAndApplySkinning(ID3D11DeviceContext* DeviceContext, const TArray<FMatrix>& FinalBoneTransforms)
{
    // --- 입력 유효성 검사 및 버퍼 맵핑 (이전과 동일) ---
    if (!DeviceContext || !DynamicVertexBuffer || BindPoseVertices.IsEmpty()) {
        return false;
    }
    if (FinalBoneTransforms.Num() != Bones.Num()) {
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = DeviceContext->Map(DynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(hr)) {
        // UE_LOG(LogTemp, Error, TEXT("Failed to map Dynamic Vertex Buffer. HRESULT: 0x%08X"), hr);
        return false;
    }

    FMeshVertex* SkinnedVertices = static_cast<FMeshVertex*>(MappedResource.pData);
    const int32 VertexCount = BindPoseVertices.Num();

    // 각 정점에 대해 CPU 스키닝 수행
    for (int32 i = 0; i < VertexCount; ++i)
    {
        const FMeshVertex& BindVertex = BindPoseVertices[i];
        FMeshVertex& SkinnedVertex = SkinnedVertices[i];

        bool bHasSignificantWeight = false;
        for (int j = 0; j < MAX_BONE_INFLUENCES; ++j) {
            if (BindVertex.BoneWeights[j] > KINDA_SMALL_NUMBER) {
                bHasSignificantWeight = true;
                break;
            }
        }

        if (!bHasSignificantWeight || Bones.IsEmpty()) {
            SkinnedVertex = BindVertex;
            continue;
        }

        FVector SkinnedPosition = FVector::ZeroVector;
        FVector SkinnedNormal = FVector::ZeroVector;
        const FVector& BindPositionVec = BindVertex.Position;
        const FVector& BindNormalVec = BindVertex.Normal;

        for (int32 j = 0; j < MAX_BONE_INFLUENCES; ++j)
        {
            float Weight = BindVertex.BoneWeights[j];
            if (Weight <= KINDA_SMALL_NUMBER) continue;

            uint32 BoneIndex = BindVertex.BoneIndices[j];
            if (!Bones.IsValidIndex(BoneIndex) || !FinalBoneTransforms.IsValidIndex(BoneIndex)) continue;

            const FMatrix& InvBindPoseMat = Bones[BoneIndex].InverseBindPoseMatrix;
            const FMatrix& FinalBoneTransformMat = FinalBoneTransforms[BoneIndex];

            const FMatrix SkinMatrix = FinalBoneTransformMat * InvBindPoseMat; // 곱셈 순서 확인!

            // --- 1. 위치 변환 ---
            // TransformPosition 멤버 함수 사용
            SkinnedPosition += SkinMatrix.TransformPosition(BindPositionVec) * Weight;

            // --- 2. 노멀 변환 (역전치 및 올바른 TransformVector 사용) ---
            FMatrix NormalMatrix = SkinMatrix;
            if (FMath::Abs(NormalMatrix.Determinant()) > KINDA_SMALL_NUMBER)
            {
                NormalMatrix = FMatrix::Inverse(NormalMatrix);      // static Inverse 사용
                NormalMatrix = FMatrix::Transpose(NormalMatrix); // static Transpose 사용

                // *** 수정된 부분: 올바른 static TransformVector 사용 ***
                FVector TransformedNormal = FMatrix::TransformVector(BindNormalVec, NormalMatrix);

                SkinnedNormal += TransformedNormal * Weight;
            }
            else
            {
                // 특이 행렬일 경우 대체 처리
                SkinnedNormal += BindNormalVec * Weight;
                // UE_LOG(LogTemp, Warning, TEXT("Vertex %d, Bone %u: SkinMatrix determinant near zero. Using bind normal."), i, BoneIndex);
            }
        } // 뼈 영향력 루프 종료

        // --- 최종 데이터 저장 ---
        SkinnedVertex.Position = SkinnedPosition;

        if (!SkinnedNormal.IsNearlyZero(KINDA_SMALL_NUMBER)) {
            SkinnedVertex.Normal = SkinnedNormal.GetSafeNormal(); // FVector의 멤버 함수 가정
        }
        else {
            SkinnedVertex.Normal = BindVertex.Normal;
        }

        SkinnedVertex.TexCoord = BindVertex.TexCoord;
        for (int k = 0; k < MAX_BONE_INFLUENCES; ++k) {
            SkinnedVertex.BoneIndices[k] = BindVertex.BoneIndices[k];
            SkinnedVertex.BoneWeights[k] = BindVertex.BoneWeights[k];
        }

    } // 정점 루프 종료

    DeviceContext->Unmap(DynamicVertexBuffer, 0);
    return true;
}
void FLoaderFBX::Render(ID3D11DeviceContext* DeviceContext)
{
    if (!DeviceContext || !DynamicVertexBuffer || !IndexBuffer || FinalIndices.IsEmpty())
    {
        // 렌더링할 준비 안 됨
        return;
    }

    // 1. 입력 조립기(Input Assembler) 단계 설정
    UINT Stride = sizeof(FMeshVertex); // 정점 하나의 크기
    UINT Offset = 0;                   // 버퍼 시작부터의 오프셋

    // 정점 버퍼 설정
    ID3D11Buffer* pVertexBuffer = DynamicVertexBuffer; // ComPtr에서 Raw 포인터 얻기
    DeviceContext->IASetVertexBuffers(0,          // 시작 슬롯 번호
        1,          // 설정할 버퍼 개수
        &pVertexBuffer, // 버퍼 배열 포인터
        &Stride,    // 스트라이드 배열 포인터
        &Offset);   // 오프셋 배열 포인터

    // 인덱스 버퍼 설정
    DeviceContext->IASetIndexBuffer(IndexBuffer, // 인덱스 버퍼 포인터
        DXGI_FORMAT_R32_UINT, // 인덱스 포맷 (uint32 사용했으므로)
        0);                   // 시작 오프셋

    // 기본 토폴로지 설정 (삼각형 리스트)
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 2. 드로우 콜 (Draw Call)
    // 인덱스를 사용하여 그리기
    DeviceContext->DrawIndexed(FinalIndices.Num(), // 그릴 인덱스 개수
        0,                  // 시작 인덱스 위치
        0);                 // 베이스 정점 위치 (Vertex Buffer 시작 기준 오프셋)
}


// --- 데이터 접근자 구현 ---
int32 FLoaderFBX::GetBoneIndex(const FName& BoneName) const
{
    const int32* FoundIndex = BoneNameToIndexMap.Find(BoneName);
    return FoundIndex ? *FoundIndex : INDEX_NONE;
}

const FBoneInfo* FLoaderFBX::GetBoneInfo(int32 BoneIndex) const
{
    if (Bones.IsValidIndex(BoneIndex))
    {
        return &Bones[BoneIndex];
    }
    return nullptr;
}


// --- 헬퍼 함수 구현 ---

// FBX Element 데이터 추출 템플릿 함수
template<typename TElementType, typename TDataType>
bool FLoaderFBX::GetElementData(
    FbxMesh* Mesh,
    TElementType* Element,
    int32 ControlPointCount,
    int32 PolygonVertexCount,
    const TArray<int32>& PolygonIndices, // 각 폴리곤 정점이 어떤 제어점을 참조하는지 알려주는 배열
    TArray<TDataType>& OutControlPointData, // MappingMode가 eByControlPoint일 때 채워짐
    TArray<TDataType>& OutPolygonVertexData) // MappingMode가 eByPolygonVertex일 때 채워짐
{
    if (!Element) return false;

    FbxGeometryElement::EMappingMode MappingMode = Element->GetMappingMode();
    FbxGeometryElement::EReferenceMode ReferenceMode = Element->GetReferenceMode();

    const auto& DirectArray = Element->GetDirectArray();
    const auto& IndexArray = Element->GetIndexArray();
    const int32 DirectDataCount = DirectArray.GetCount();

    // 기본값 설정 (TDataType에 따라 다름)
    TDataType DefaultValue = {};
    if constexpr (std::is_same_v<TDataType, FVector>) DefaultValue = FVector(0, 0, 1); // 노멀 기본값
    else if constexpr (std::is_same_v<TDataType, FVector2D>) DefaultValue = FVector2D(0, 0); // UV 기본값

    auto GetDataValue = [&](int DataIndex) -> TDataType {
        if (DataIndex >= 0 && DataIndex < DirectDataCount) {
            if constexpr (std::is_same_v<TDataType, FVector>) {
                // FbxDouble3 또는 FbxVector4를 FVector로 변환
                return ConvertFbxNormal(DirectArray.GetAt(DataIndex));
            }
            else if constexpr (std::is_same_v<TDataType, FVector2D>) {
                // FbxDouble2 또는 FbxVector2를 FVector2D로 변환
                return ConvertFbxUV(DirectArray.GetAt(DataIndex));
            }
        }
        // UE_LOG(LogLevel::Warning, TEXT("Invalid data index %d requested (DirectDataCount: %d). Returning default."), DataIndex, DirectDataCount);
        return DefaultValue;
        };

    if (MappingMode == FbxGeometryElement::eByControlPoint)
    {
        OutControlPointData.SetNum(ControlPointCount); // 제어점 개수만큼 배열 크기 설정
        if (ReferenceMode == FbxGeometryElement::eDirect)
        {
            if (DirectDataCount != ControlPointCount) {
                // UE_LOG(LogLevel::Warning, TEXT("eByControlPoint/eDirect data count mismatch. Expected %d, Got %d."), ControlPointCount, DirectDataCount);
                // 크기가 안 맞아도 일단 채움, 부족하면 기본값
            }
            for (int32 i = 0; i < ControlPointCount; ++i) {
                OutControlPointData[i] = GetDataValue(i);
            }
            return true;
        }
        else if (ReferenceMode == FbxGeometryElement::eIndexToDirect)
        {
            const int32 IndexArrayCount = IndexArray.GetCount();
            if (IndexArrayCount != ControlPointCount) {
                // UE_LOG(LogLevel::Warning, TEXT("eByControlPoint/eIndexToDirect index count mismatch. Expected %d, Got %d."), ControlPointCount, IndexArrayCount);
            }
            for (int32 i = 0; i < ControlPointCount; ++i) {
                if (i < IndexArrayCount) {
                    int32 DataIndex = IndexArray.GetAt(i);
                    OutControlPointData[i] = GetDataValue(DataIndex);
                }
                else {
                    OutControlPointData[i] = DefaultValue; // 인덱스 배열 부족 시 기본값
                }
            }
            return true;
        }
    }
    else if (MappingMode == FbxGeometryElement::eByPolygonVertex)
    {
        OutPolygonVertexData.SetNum(PolygonVertexCount); // 폴리곤 정점 개수만큼 배열 크기 설정
        if (ReferenceMode == FbxGeometryElement::eDirect)
        {
            if (DirectDataCount != PolygonVertexCount) {
                // UE_LOG(LogLevel::Warning, TEXT("eByPolygonVertex/eDirect data count mismatch. Expected %d, Got %d."), PolygonVertexCount, DirectDataCount);
            }
            for (int32 i = 0; i < PolygonVertexCount; ++i) {
                OutPolygonVertexData[i] = GetDataValue(i);
            }
            return true;
        }
        else if (ReferenceMode == FbxGeometryElement::eIndexToDirect)
        {
            const int32 IndexArrayCount = IndexArray.GetCount();
            for (int32 i = 0; i < PolygonVertexCount; ++i) {
                if (i < IndexArrayCount) {
                    int32 DataIndex = IndexArray.GetAt(i);
                    OutPolygonVertexData[i] = GetDataValue(DataIndex);
                }
                else {
                    OutPolygonVertexData[i] = DefaultValue;
                }
            }
            return true;
        }
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("Unsupported MappingMode: %d"), static_cast<int32>(MappingMode));
    }

    return false; // 처리 실패 또는 지원하지 않는 모드
}


// --- 좌표계 변환 헬퍼 구현 ---
FVector FLoaderFBX::ConvertFbxPosition(const FbxVector4& Vector)
{
    // SDK에서 이미 목표 좌표계로 변환됨. 축 스와핑 불필요.
    // 필요시 스케일 변환(예: cm -> m)만 적용.
    return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
}

FVector FLoaderFBX::ConvertFbxNormal(const FbxVector4& Vector)
{
    // SDK에서 이미 목표 좌표계로 변환됨. 축 스와핑 불필요.
    return FVector(static_cast<float>(Vector[0]), static_cast<float>(Vector[1]), static_cast<float>(Vector[2]));
}
FVector2D FLoaderFBX::ConvertFbxUV(const FbxVector2& Vector)
{
    return FVector2D(static_cast<float>(Vector[0]), 1.0f - static_cast<float>(Vector[1]));
}
