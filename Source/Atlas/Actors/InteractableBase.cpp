#include "InteractableBase.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
// #include "../Core/AtlasGameplayTags.h" // TODO: Fix AtlasGameplayTags compilation
#include "TimerManager.h"
#include "Engine/World.h"

AInteractableBase::AInteractableBase()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    FocusedParticleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FocusedParticleEffect"));
    FocusedParticleEffect->SetupAttachment(MeshComponent);
    FocusedParticleEffect->bAutoActivate = false;
}

void AInteractableBase::BeginPlay()
{
    Super::BeginPlay();
    
    // StateTags.AddTag(FAtlasGameplayTags::Get().Interactable_State_Ready); // TODO: Fix AtlasGameplayTags compilation
    
    if (bStartOnCooldown)
    {
        StartCooldown();
    }
    
    UpdateVisualState();
}

void AInteractableBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsOnCooldown && CooldownTimeRemaining > 0)
    {
        CooldownTimeRemaining -= DeltaTime;
        UpdateCooldownVisuals(CooldownTimeRemaining, CooldownDuration);
    }
}

FGameplayTag AInteractableBase::GetInteractableType_Implementation() const
{
    return InteractableTypeTag;
}

bool AInteractableBase::CanInteract_Implementation(AActor* Interactor) const
{
    return !bIsOnCooldown; // TODO: Re-enable tag check: && StateTags.HasTag(FAtlasGameplayTags::Get().Interactable_State_Ready);
}

void AInteractableBase::OnFocused_Implementation(AActor* FocusingActor)
{
    bIsFocused = true;
    
    if (FocusedParticleEffect)
    {
        FocusedParticleEffect->Activate();
    }
    
    UpdateFocusVisuals(true);
    UpdateVisualState();
    
    UE_LOG(LogTemp, Log, TEXT("%s is now focused by %s"), *GetName(), *FocusingActor->GetName());
}

void AInteractableBase::OnUnfocused_Implementation(AActor* FocusingActor)
{
    bIsFocused = false;
    
    if (FocusedParticleEffect)
    {
        FocusedParticleEffect->Deactivate();
    }
    
    UpdateFocusVisuals(false);
    UpdateVisualState();
    
    UE_LOG(LogTemp, Log, TEXT("%s is no longer focused"), *GetName());
}

void AInteractableBase::Interact_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Warning, TEXT("Interactable: %s, Interactor: %s"), *GetName(), Interactor ? *Interactor->GetName() : TEXT("None"));
    
    if (!CanInteract_Implementation(Interactor))
    {
        UE_LOG(LogTemp, Warning, TEXT("CanInteract returned false"));
        OnInteractionFailed(Interactor);
        return;
    }
    
    ExecuteInteraction(Interactor);
    OnInteractionSuccessful(Interactor);
    OnInteracted.Broadcast(Interactor);
    
    StartCooldown();
}

float AInteractableBase::GetInteractionDistance_Implementation() const
{
    return MaxInteractionDistance;
}

FVector AInteractableBase::GetFocusLocation_Implementation() const
{
    if (MeshComponent)
    {
        return MeshComponent->GetComponentLocation() + FVector(0, 0, 50);
    }
    return GetActorLocation();
}

bool AInteractableBase::IsOnCooldown_Implementation() const
{
    return bIsOnCooldown;
}

float AInteractableBase::GetCooldownRemaining_Implementation() const
{
    return CooldownTimeRemaining;
}

void AInteractableBase::StartCooldown()
{
    if (bIsOnCooldown)
    {
        return;
    }
    
    bIsOnCooldown = true;
    CooldownTimeRemaining = CooldownDuration;
    
    // StateTags.RemoveTag(FAtlasGameplayTags::Get().Interactable_State_Ready); // TODO: Fix AtlasGameplayTags compilation
    // StateTags.AddTag(FAtlasGameplayTags::Get().Interactable_State_Cooldown); // TODO: Fix AtlasGameplayTags compilation
    
    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimerHandle,
        this,
        &AInteractableBase::EndCooldown,
        CooldownDuration,
        false
    );
    
    OnCooldownStarted.Broadcast();
    UpdateVisualState();
    
    UE_LOG(LogTemp, Log, TEXT("%s started cooldown for %.1f seconds"), *GetName(), CooldownDuration);
}

void AInteractableBase::EndCooldown()
{
    bIsOnCooldown = false;
    CooldownTimeRemaining = 0.0f;
    
    // StateTags.RemoveTag(FAtlasGameplayTags::Get().Interactable_State_Cooldown); // TODO: Fix AtlasGameplayTags compilation
    // StateTags.AddTag(FAtlasGameplayTags::Get().Interactable_State_Ready); // TODO: Fix AtlasGameplayTags compilation
    
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
    
    OnCooldownEnded.Broadcast();
    UpdateVisualState();
    
    UE_LOG(LogTemp, Log, TEXT("%s cooldown ended"), *GetName());
}

void AInteractableBase::UpdateVisualState()
{
    if (MeshComponent)
    {
        if (bIsOnCooldown)
        {
            MeshComponent->SetScalarParameterValueOnMaterials(TEXT("Emissive"), 0.1f);
        }
        else if (bIsFocused)
        {
            MeshComponent->SetScalarParameterValueOnMaterials(TEXT("Emissive"), 2.0f);
        }
        else
        {
            MeshComponent->SetScalarParameterValueOnMaterials(TEXT("Emissive"), 1.0f);
        }
    }
}

void AInteractableBase::ExecuteInteraction(AActor* Interactor)
{
}