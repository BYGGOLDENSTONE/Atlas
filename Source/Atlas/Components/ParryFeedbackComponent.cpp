#include "ParryFeedbackComponent.h"
#include "CombatComponent.h"
#include "VulnerabilityComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

UParryFeedbackComponent::UParryFeedbackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UParryFeedbackComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetupEventBindings();
}

void UParryFeedbackComponent::SetupEventBindings()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    CombatComponent = Owner->FindComponentByClass<UCombatComponent>();
    if (CombatComponent)
    {
        CombatComponent->OnParrySuccess.AddDynamic(this, &UParryFeedbackComponent::OnParrySuccess);
    }

    VulnerabilityComponent = Owner->FindComponentByClass<UVulnerabilityComponent>();
    if (VulnerabilityComponent)
    {
        VulnerabilityComponent->OnVulnerabilityApplied.AddDynamic(this, &UParryFeedbackComponent::OnVulnerabilityStateApplied);
        VulnerabilityComponent->OnVulnerabilityEnded.AddDynamic(this, &UParryFeedbackComponent::OnVulnerabilityEnded);
        VulnerabilityComponent->OnIFramesStarted.AddDynamic(this, &UParryFeedbackComponent::OnIFramesStarted);
    }
}

void UParryFeedbackComponent::OnParrySuccess(AActor* Attacker)
{
    PlayParrySuccessFeedback(Attacker);
}

void UParryFeedbackComponent::OnVulnerabilityStateApplied(int32 Charges, float Duration)
{
    PlayVulnerabilityAppliedFeedback();
}

void UParryFeedbackComponent::OnVulnerabilityEnded()
{
    PlayVulnerabilityEndedFeedback();
}

void UParryFeedbackComponent::OnIFramesStarted()
{
    PlayIFrameFeedback();
}

void UParryFeedbackComponent::PlayParrySuccessFeedback(AActor* Attacker)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector Location = Owner->GetActorLocation() + ParryVFXOffset;

    if (ParrySuccessVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParrySuccessVFX,
            Location,
            FRotator::ZeroRotator,
            FVector(ParryVFXScale)
        );
    }

    if (ParrySuccessSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ParrySuccessSound,
            Location,
            ParrySuccessSoundVolume
        );
    }

    if (ParrySuccessCameraShake)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraShake(ParrySuccessCameraShake);
        }
    }

    TriggerSlowMotion();

    UE_LOG(LogTemp, Warning, TEXT("Parry Success Feedback Triggered!"));
}

void UParryFeedbackComponent::PlayVulnerabilityAppliedFeedback()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector Location = Owner->GetActorLocation() + VulnerabilityVFXOffset;

    if (VulnerabilityVFX && !ActiveVulnerabilityVFX)
    {
        ActiveVulnerabilityVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VulnerabilityVFX,
            Owner->GetRootComponent(),
            NAME_None,
            VulnerabilityVFXOffset,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false,
            true,
            ENCPoolMethod::None,
            true
        );
        
        if (ActiveVulnerabilityVFX)
        {
            ActiveVulnerabilityVFX->SetWorldScale3D(FVector(VulnerabilityVFXScale));
        }
    }

    if (VulnerabilityAppliedSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            VulnerabilityAppliedSound,
            Location
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Vulnerability Applied Feedback!"));
}

void UParryFeedbackComponent::PlayVulnerabilityEndedFeedback()
{
    if (ActiveVulnerabilityVFX)
    {
        ActiveVulnerabilityVFX->DestroyComponent();
        ActiveVulnerabilityVFX = nullptr;
    }

    if (VulnerabilityEndedSound)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                VulnerabilityEndedSound,
                Owner->GetActorLocation()
            );
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Vulnerability Ended Feedback!"));
}

void UParryFeedbackComponent::PlayIFrameFeedback()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (IFrameVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            IFrameVFX,
            Owner->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    if (IFrameSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            IFrameSound,
            Owner->GetActorLocation()
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("I-Frame Feedback!"));
}

void UParryFeedbackComponent::TriggerSlowMotion()
{
    if (SlowMotionDuration <= 0.0f)
    {
        return;
    }

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionScale);

    GetWorld()->GetTimerManager().SetTimer(
        SlowMotionTimerHandle,
        this,
        &UParryFeedbackComponent::EndSlowMotion,
        SlowMotionDuration * SlowMotionScale,
        false
    );
}

void UParryFeedbackComponent::EndSlowMotion()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}