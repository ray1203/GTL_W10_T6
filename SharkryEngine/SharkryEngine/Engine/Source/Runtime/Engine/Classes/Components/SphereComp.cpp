
#include "SphereComp.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"

USphereComp::USphereComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = {1, 1, 1};
    AABB.min = {-1, -1, -1};
}

void USphereComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
