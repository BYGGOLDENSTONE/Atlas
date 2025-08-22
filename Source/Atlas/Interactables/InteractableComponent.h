// InteractableComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Interfaces/IInteractable.h"
#include "GameplayTagContainer.h"
#include "InteractableComponent.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UMaterialInterface;
class USoundCue;

UENUM(BlueprintType)
enum class EInteractableState : uint8
{
    Ready         UMETA(DisplayName = "Ready"),
    Interacting   UMETA(DisplayName = "Interacting"),
    Cooldown      UMETA(DisplayName = "On Cooldown"),
    Disabled      UMETA(DisplayName = "Disabled"),
    Depleted      UMETA(DisplayName = "Depleted")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStarted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCancelled, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UInteractableComponent : public UActorComponent, public IInteractable
{
    GENERATED_BODY()

public:
    UInteractableComponent();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    FGameplayTag InteractableTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    float InteractionRange = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    float InteractionTime = 0.0f; // 0 = instant
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    float Cooldown = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    int32 MaxUses = -1; // -1 = unlimited
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    bool bRequiresLineOfSight = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    bool bAutoDetectNearbyPlayers = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    float AutoDetectRadius = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    FText InteractionPromptText;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    FText DisabledPromptText;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    FText CooldownPromptText;
    
    // Visual feedback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMeshComponent* InteractableMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UWidgetComponent* InteractionPrompt;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UMaterialInterface* ReadyMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UMaterialInterface* InteractingMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UMaterialInterface* CooldownMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UMaterialInterface* DisabledMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    bool bShowInteractionProgress = true;
    
    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* InteractionStartSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* InteractionCompleteSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* InteractionCancelSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* CooldownEndSound;
    
    // IInteractable interface implementations
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual FGameplayTag GetInteractableType_Implementation() const override { return InteractableTag; }
    virtual void OnFocused_Implementation(AActor* FocusingActor) override {}
    virtual void OnUnfocused_Implementation(AActor* FocusingActor) override {}
    virtual float GetInteractionDistance_Implementation() const override { return InteractionRange; }
    virtual FVector GetFocusLocation_Implementation() const override;
    virtual bool IsOnCooldown_Implementation() const override { return CurrentCooldown > 0.0f; }
    virtual float GetCooldownRemaining_Implementation() const override { return CurrentCooldown; }
    
    // Additional helper functions
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    FText GetInteractionPrompt();
    
    // State management
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void SetInteractableEnabled(bool bEnabled);
    
    UFUNCTION(BlueprintPure, Category = "Interactable")
    bool IsInteractableEnabled() const { return bIsEnabled; }
    
    
    UFUNCTION(BlueprintPure, Category = "Interactable")
    float GetCooldownPercent() const;
    
    UFUNCTION(BlueprintPure, Category = "Interactable")
    int32 GetUsesRemaining() const;
    
    UFUNCTION(BlueprintPure, Category = "Interactable")
    EInteractableState GetCurrentState() const { return CurrentState; }
    
    UFUNCTION(BlueprintPure, Category = "Interactable")
    float GetInteractionProgress() const { return InteractionProgress; }
    
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void CancelInteraction();
    
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void ResetInteractable();
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Interactable|Events")
    FOnInteractionStarted OnInteractionStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable|Events")
    FOnInteractionCompleted OnInteractionCompleted;
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable|Events")
    FOnInteractionCancelled OnInteractionCancelled;
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable|Events")
    FOnInteractableStateChanged OnStateChanged;
    
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnInteractionStartedBP(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnInteractionCompletedBP(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnInteractionCancelledBP(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnCooldownEnded();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Override in derived classes for custom interaction logic
    virtual void ExecuteInteraction(AActor* Interactor);
    virtual void UpdateVisualState();
    virtual bool ValidateInteractor(AActor* Interactor) const;
    
    // Helper functions
    void StartInteraction(AActor* Interactor);
    void CompleteInteraction();
    void StartCooldown();
    void EndCooldown();
    void UpdateInteractionProgress(float DeltaTime);
    void UpdatePromptVisibility();
    void PlayInteractionSound(USoundCue* Sound);
    void SetMeshMaterial(UMaterialInterface* Material);
    
private:
    EInteractableState CurrentState;
    float CurrentCooldown;
    int32 CurrentUses;
    bool bIsEnabled;
    bool bIsBeingInteracted;
    float InteractionProgress;
    
    UPROPERTY()
    AActor* CurrentInteractor;
    
    FTimerHandle CooldownTimerHandle;
    FTimerHandle InteractionTimerHandle;
    FTimerHandle PromptUpdateTimerHandle;
    
    void CheckNearbyPlayers();
    float GetDistanceToNearestPlayer() const;
};