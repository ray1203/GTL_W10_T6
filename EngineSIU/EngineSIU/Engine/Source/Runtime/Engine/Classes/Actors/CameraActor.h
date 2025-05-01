#pragma once

#include "GameFramework/Actor.h"

class ACameraActor : public AActor
{
    DECLARE_CLASS(ACameraActor, AActor)

public:
    ACameraActor();

    virtual void BeginPlay() override;

private:
    /** The camera component for this camera */
    class UCameraComponent* CameraComponent = nullptr;
    class USceneComponent* SceneComponent = nullptr;

public:
    /** Returns CameraComponent subobject **/
    class UCameraComponent* GetCameraComponent() const { return CameraComponent; }


};

