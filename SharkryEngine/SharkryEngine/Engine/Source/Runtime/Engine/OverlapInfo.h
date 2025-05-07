class UPrimitiveComponent;
class AActor;

struct FOverlapInfo
{
    UPrimitiveComponent* OtherComponent;
    AActor* OtherActor;

    FOverlapInfo(UPrimitiveComponent* InComponent, AActor* InActor)
        : OtherComponent(InComponent), OtherActor(InActor) {
    }
};
