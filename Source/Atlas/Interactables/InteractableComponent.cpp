// InteractableComponent.cpp
#include "InteractableComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UInteractableComponent::UInteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentState = EInteractableState::Ready;
    CurrentCooldown = 0.0f;
    CurrentUses = 0;
    bIsEnabled = true;
    bIsBeingInteracted = false;
    InteractionProgress = 0.0f;
    CurrentInteractor = nullptr;
    
    // Default texts
    InteractionPromptText = FText::FromString("Press [E] to Interact");
    DisabledPromptText = FText::FromString("Disabled");
    CooldownPromptText = FText::FromString("Recharging...");
}

void UInteractableComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup visual state
    UpdateVisualState();
    
    // Start checking for nearby players if configured
    if (bAutoDetectNearbyPlayers)
    {
        GetWorld()->GetTimerManager().SetTimer(PromptUpdateTimerHandle, this,
            &UInteractableComponent::CheckNearbyPlayers, 0.5f, true);
    }
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(PromptUpdateTimerHandle);
    
    Super::EndPlay(EndPlayReason);
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cooldown
    if (CurrentCooldown > 0.0f)
    {
        CurrentCooldown -= DeltaTime;
        if (CurrentCooldown <= 0.0f)
        {
            EndCooldown();
        }
    }
    
    // Update interaction progress
    if (bIsBeingInteracted && InteractionTime > 0.0f)
    {
        UpdateInteractionProgress(DeltaTime);
    }
    
    // Update prompt visibility based on distance
    UpdatePromptVisibility();
}

bool UInteractableComponent::CanInteract_Implementation(AActor* Interactor) const
{
    if (!bIsEnabled || !Interactor)
    {
        return false;
    }
    
    // Check state
    if (CurrentState != EInteractableState::Ready)
    {
        return false;
    }
    
    // Check uses
    if (MaxUses > 0 && CurrentUses >= MaxUses)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Interactor->GetActorLocation());
    if (Distance > InteractionRange)
    {
        return false;
    }
    
    // Check line of sight
    if (bRequiresLineOfSight)
    {
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.AddIgnoredActor(Interactor);
        
        bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Interactor->GetActorLocation() + FVector(0, 0, 50), // Eye height offset
            GetOwner()->GetActorLocation(),
            ECC_Visibility,
            QueryParams
        );
        
        if (!bHasLineOfSight)
        {
            return false;
        }
    }
    
    // Additional validation
    return ValidateInteractor(Interactor);
}

void UInteractableComponent::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor))
    {
        return;
    }
    
    CurrentInteractor = Interactor;
    
    if (InteractionTime <= 0.0f)
    {
        // Instant interaction
        StartInteraction(Interactor);
        ExecuteInteraction(Interactor);
        CompleteInteraction();
    }
    else
    {
        // Timed interaction
        StartInteraction(Interactor);
        
        // Set timer for completion
        GetWorld()->GetTimerManager().SetTimer(InteractionTimerHandle, this,
            &UInteractableComponent::CompleteInteraction, InteractionTime, false);
    }
}

FText UInteractableComponent::GetInteractionPrompt()
{
    switch (CurrentState)
    {
        case EInteractableState::Ready:
            return InteractionPromptText;
        case EInteractableState::Cooldown:
            return FText::Format(CooldownPromptText, FText::AsNumber(FMath::CeilToInt(CurrentCooldown)));
        case EInteractableState::Disabled:
        case EInteractableState::Depleted:
            return DisabledPromptText;
        case EInteractableState::Interacting:
            return FText::FromString("Interacting...");
        default:
            return FText::GetEmpty();
    }
}

void UInteractableComponent::SetInteractableEnabled(bool bEnabled)
{
    if (bIsEnabled != bEnabled)
    {
        bIsEnabled = bEnabled;
        CurrentState = bEnabled ? EInteractableState::Ready : EInteractableState::Disabled;
        UpdateVisualState();
        OnStateChanged.Broadcast();
    }
}

FVector UInteractableComponent::GetFocusLocation_Implementation() const
{
    return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

float UInteractableComponent::GetCooldownPercent() const
{
    if (Cooldown <= 0.0f)
    {
        return 0.0f;
    }
    
    return CurrentCooldown / Cooldown;
}

int32 UInteractableComponent::GetUsesRemaining() const
{
    if (MaxUses <= 0)
    {
        return -1; // Unlimited
    }
    
    return FMath::Max(0, MaxUses - CurrentUses);
}

void UInteractableComponent::CancelInteraction()
{
    if (!bIsBeingInteracted)
    {
        return;
    }
    
    bIsBeingInteracted = false;
    InteractionProgress = 0.0f;
    CurrentState = EInteractableState::Ready;
    
    GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
    
    PlayInteractionSound(InteractionCancelSound);
    UpdateVisualState();
    
    OnInteractionCancelled.Broadcast(CurrentInteractor);
    OnInteractionCancelledBP(CurrentInteractor);
    
    CurrentInteractor = nullptr;
    OnStateChanged.Broadcast();
}

void UInteractableComponent::ResetInteractable()
{
    CurrentCooldown = 0.0f;
    CurrentUses = 0;
    bIsBeingInteracted = false;
    InteractionProgress = 0.0f;
    CurrentInteractor = nullptr;
    CurrentState = bIsEnabled ? EInteractableState::Ready : EInteractableState::Disabled;
    
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
    
    UpdateVisualState();
    OnStateChanged.Broadcast();
}

void UInteractableComponent::ExecuteInteraction(AActor* Interactor)
{
    // Override in derived classes for specific interaction logic
    // Base implementation just increments use count
    CurrentUses++;
}

void UInteractableComponent::UpdateVisualState()
{
    UMaterialInterface* MaterialToUse = nullptr;
    
    switch (CurrentState)
    {
        case EInteractableState::Ready:
            MaterialToUse = ReadyMaterial;
            break;
        case EInteractableState::Interacting:
            MaterialToUse = InteractingMaterial;
            break;
        case EInteractableState::Cooldown:
            MaterialToUse = CooldownMaterial;
            break;
        case EInteractableState::Disabled:
        case EInteractableState::Depleted:
            MaterialToUse = DisabledMaterial;
            break;
    }
    
    if (MaterialToUse)
    {
        SetMeshMaterial(MaterialToUse);
    }
    
    // Update widget prompt
    if (InteractionPrompt)
    {
        InteractionPrompt->SetVisibility(CurrentState == EInteractableState::Ready);
    }
}

bool UInteractableComponent::ValidateInteractor(AActor* Interactor) const
{
    // Override in derived classes for additional validation
    return IsValid(Interactor);
}

void UInteractableComponent::StartInteraction(AActor* Interactor)
{
    bIsBeingInteracted = true;
    InteractionProgress = 0.0f;
    CurrentState = EInteractableState::Interacting;
    
    PlayInteractionSound(InteractionStartSound);
    UpdateVisualState();
    
    OnInteractionStarted.Broadcast(Interactor);
    OnInteractionStartedBP(Interactor);
    OnStateChanged.Broadcast();
}

void UInteractableComponent::CompleteInteraction()
{
    if (!bIsBeingInteracted)
    {
        return;
    }
    
    bIsBeingInteracted = false;
    InteractionProgress = 1.0f;
    
    // Execute the actual interaction
    if (CurrentInteractor)
    {
        ExecuteInteraction(CurrentInteractor);
    }
    
    PlayInteractionSound(InteractionCompleteSound);
    
    OnInteractionCompleted.Broadcast(CurrentInteractor);
    OnInteractionCompletedBP(CurrentInteractor);
    
    // Check if depleted
    if (MaxUses > 0 && CurrentUses >= MaxUses)
    {
        CurrentState = EInteractableState::Depleted;
        bIsEnabled = false;
    }
    else
    {
        // Start cooldown
        StartCooldown();
    }
    
    CurrentInteractor = nullptr;
    UpdateVisualState();
    OnStateChanged.Broadcast();
}

void UInteractableComponent::StartCooldown()
{
    if (Cooldown <= 0.0f)
    {
        CurrentState = EInteractableState::Ready;
        return;
    }
    
    CurrentCooldown = Cooldown;
    CurrentState = EInteractableState::Cooldown;
    
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this,
        &UInteractableComponent::EndCooldown, Cooldown, false);
    
    UpdateVisualState();
}

void UInteractableComponent::EndCooldown()
{
    CurrentCooldown = 0.0f;
    CurrentState = EInteractableState::Ready;
    
    PlayInteractionSound(CooldownEndSound);
    UpdateVisualState();
    
    OnCooldownEnded();
    OnStateChanged.Broadcast();
}

void UInteractableComponent::UpdateInteractionProgress(float DeltaTime)
{
    if (!bIsBeingInteracted || InteractionTime <= 0.0f)
    {
        return;
    }
    
    InteractionProgress = FMath::Clamp(InteractionProgress + (DeltaTime / InteractionTime), 0.0f, 1.0f);
    
    // Update visual feedback
    if (bShowInteractionProgress && InteractionPrompt)
    {
        // This would update a progress bar in the widget
        // Implementation depends on the widget blueprint
    }
    
    // Check if interactor moved too far away
    if (CurrentInteractor)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentInteractor->GetActorLocation());
        if (Distance > InteractionRange * 1.5f) // Give some leeway
        {
            CancelInteraction();
        }
    }
}

void UInteractableComponent::UpdatePromptVisibility()
{
    if (!InteractionPrompt || !bAutoDetectNearbyPlayers)
    {
        return;
    }
    
    float DistanceToPlayer = GetDistanceToNearestPlayer();
    bool bShouldShowPrompt = (DistanceToPlayer <= AutoDetectRadius) && 
                             (CurrentState == EInteractableState::Ready || CurrentState == EInteractableState::Cooldown);
    
    InteractionPrompt->SetVisibility(bShouldShowPrompt);
    
    // Update prompt text
    if (bShouldShowPrompt)
    {
        // Update the text based on current state
        // This would be done through the widget blueprint
    }
}

void UInteractableComponent::PlayInteractionSound(USoundCue* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), (USoundBase*)Sound, GetOwner()->GetActorLocation());
    }
}

void UInteractableComponent::SetMeshMaterial(UMaterialInterface* Material)
{
    if (InteractableMesh && Material)
    {
        InteractableMesh->SetMaterial(0, Material);
    }
}

void UInteractableComponent::CheckNearbyPlayers()
{
    UpdatePromptVisibility();
}

float UInteractableComponent::GetDistanceToNearestPlayer() const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
    }
    
    return FLT_MAX;
}