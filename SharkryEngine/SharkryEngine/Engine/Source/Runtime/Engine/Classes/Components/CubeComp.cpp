#include "CubeComp.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"


UCubeComp::UCubeComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = { 1,1,1 };
    AABB.min = { -1,-1,-1 };

}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();

    //FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    //SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
    // 
    // Begin Test
    FManagerOBJ::CreateStaticMesh("Contents/Reference/Reference.obj");
    SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Reference.obj"));
    // End Test
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}
