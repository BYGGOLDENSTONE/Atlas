#include "AtlasGameplayTags.h"
#include "GameplayTagsManager.h"

FAtlasGameplayTags FAtlasGameplayTags::GameplayTags;

void FAtlasGameplayTags::InitializeNativeTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // COMMENTED OUT - We're using tags from DefaultGameplayTags.ini instead
    // Keeping this file for potential future native tag needs
    
    /*
    // ===== OLD NATIVE TAGS - REMOVED =====
    // These were causing conflicts with our clean tag structure
    // All tags are now defined in Config/DefaultGameplayTags.ini
    
    // If we need native tags in the future, use this pattern:
    // GameplayTags.SomeTag = Manager.AddNativeGameplayTag(FName("Category.Subcategory.Tag"), FString("Description"));
    */
    
    // Currently no native tags needed - all tags come from DefaultGameplayTags.ini
}