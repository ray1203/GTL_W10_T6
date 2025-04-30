#pragma once

#include "GameFramework/Actor.h"

class ACameraActor : public AActor
{
    DECLARE_CLASS(ACameraActor, AActor)

public:
    ACameraActor() = default;


private:
    /** The camera component for this camera */
    class UCameraComponent* CameraComponent;
    class USceneComponent* SceneComponent;

public:
    /** Returns CameraComponent subobject **/
    class UCameraComponent* GetCameraComponent() const { return CameraComponent; }


};

