#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "IInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class ATLAS_API IInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    FGameplayTag GetInteractableType() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool CanInteract(AActor* Interactor) const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void OnFocused(AActor* FocusingActor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void OnUnfocused(AActor* FocusingActor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    float GetInteractionDistance() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    FVector GetFocusLocation() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsOnCooldown() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    float GetCooldownRemaining() const;
};