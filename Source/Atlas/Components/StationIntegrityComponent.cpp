#include "StationIntegrityComponent.h"
#include "../Data/StationIntegrityDataAsset.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

UStationIntegrityComponent::UStationIntegrityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UStationIntegrityComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (IntegrityDataAsset)
    {
        MaxIntegrity = IntegrityDataAsset->MaxIntegrity;
    }
    
    CurrentIntegrity = MaxIntegrity;
}

void UStationIntegrityComponent::ApplyIntegrityDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (bIsIntegrityFailed || DamageAmount <= 0.0f)
    {
        return;
    }

    float OldIntegrity = CurrentIntegrity;
    CurrentIntegrity = FMath::Clamp(CurrentIntegrity - DamageAmount, 0.0f, MaxIntegrity);
    float ActualDamage = OldIntegrity - CurrentIntegrity;

    if (ActualDamage > 0.0f)
    {
        OnIntegrityDamaged.Broadcast(ActualDamage, DamageInstigator);
        BroadcastIntegrityChange(-ActualDamage);
        CheckIntegrityThresholds();
    }
}

void UStationIntegrityComponent::RestoreIntegrity(float RestoreAmount)
{
    if (bIsIntegrityFailed || RestoreAmount <= 0.0f)
    {
        return;
    }

    float OldIntegrity = CurrentIntegrity;
    CurrentIntegrity = FMath::Clamp(CurrentIntegrity + RestoreAmount, 0.0f, MaxIntegrity);
    float ActualRestore = CurrentIntegrity - OldIntegrity;

    if (ActualRestore > 0.0f)
    {
        BroadcastIntegrityChange(ActualRestore);
        CheckIntegrityThresholds();
    }
}

void UStationIntegrityComponent::SetIntegrity(float NewIntegrity)
{
    if (bIsIntegrityFailed)
    {
        return;
    }

    float OldIntegrity = CurrentIntegrity;
    CurrentIntegrity = FMath::Clamp(NewIntegrity, 0.0f, MaxIntegrity);
    float IntegrityDelta = CurrentIntegrity - OldIntegrity;

    if (!FMath::IsNearlyZero(IntegrityDelta))
    {
        BroadcastIntegrityChange(IntegrityDelta);
        CheckIntegrityThresholds();
    }
}

void UStationIntegrityComponent::SetIntegrityPercent(float Percent)
{
    float NewIntegrity = (Percent / 100.0f) * MaxIntegrity;
    SetIntegrity(NewIntegrity);
}

float UStationIntegrityComponent::GetIntegrityPercent() const
{
    if (MaxIntegrity <= 0.0f)
    {
        return 0.0f;
    }
    return (CurrentIntegrity / MaxIntegrity) * 100.0f;
}


void UStationIntegrityComponent::CheckIntegrityThresholds()
{
    float IntegrityPercent = GetIntegrityPercent();
    
    if (IntegrityPercent <= FailureThreshold && !bIsIntegrityFailed)
    {
        bIsIntegrityFailed = true;
        HandleIntegrityFailure();
        OnIntegrityFailed.Broadcast();
        OnIntegrityThresholdReached.Broadcast(0.0f);
    }
    else if (IntegrityPercent <= CriticalThreshold && !bIsIntegrityCritical)
    {
        bIsIntegrityCritical = true;
        OnIntegrityCritical.Broadcast();
        OnIntegrityThresholdReached.Broadcast(50.0f);
    }
    else if (IntegrityPercent > CriticalThreshold && bIsIntegrityCritical)
    {
        bIsIntegrityCritical = false;
    }
}

void UStationIntegrityComponent::HandleIntegrityFailure()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Station Integrity Failed! Game Over!"));
    }
}

void UStationIntegrityComponent::BroadcastIntegrityChange(float IntegrityDelta)
{
    OnIntegrityChanged.Broadcast(CurrentIntegrity, MaxIntegrity, IntegrityDelta);
}

void UStationIntegrityComponent::ApplyAbilityIntegrityCost(const FGameplayTag& AbilityTag, AActor* Instigator)
{
    if (!IntegrityDataAsset)
    {
        return;
    }
    
    float IntegrityCost = IntegrityDataAsset->GetIntegrityCostForAbility(AbilityTag);
    if (IntegrityCost > 0.0f)
    {
        ApplyIntegrityDamage(IntegrityCost, Instigator);
    }
}