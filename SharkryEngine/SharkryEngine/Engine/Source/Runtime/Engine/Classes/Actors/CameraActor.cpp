#include "CameraActor.h"

#include "Camera/CameraComponent.h"

ACameraActor::ACameraActor()
{
    if (!CameraComponent)
    {
        CameraComponent = AddComponent<UCameraComponent>("Camera");
        SetRootComponent(CameraComponent);
    }
}

void ACameraActor::BeginPlay()
{
    Super::BeginPlay();

}
