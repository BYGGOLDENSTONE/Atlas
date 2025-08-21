#include "ActionDataAsset.h"

#if WITH_EDITOR
void UActionDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Auto-set action class based on type (optional)
    if (PropertyChangedEvent.Property && 
        PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UActionDataAsset, ActionType))
    {
        // You could auto-set the ActionClass here based on ActionType
        // For now, we'll let the user choose
    }
    
    // Auto-suggest tags based on type (optional)
    if (PropertyChangedEvent.Property && 
        PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UActionDataAsset, ActionTag))
    {
        // Could validate or suggest appropriate tags
    }
}
#endif