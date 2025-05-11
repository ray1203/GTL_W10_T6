#include "AnimSequenceBase.h"

UAnimSequenceBase::UAnimSequenceBase()
{
}

UAnimDataModel* UAnimSequenceBase::GetDataModel() const
{
    Super::GetDataModel();

    return DataModel;
}
