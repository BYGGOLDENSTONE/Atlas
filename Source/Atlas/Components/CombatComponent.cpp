#include "CombatComponent.h"
#include "../Data/AttackDataAsset.h"
#include "../DataAssets/ActionDataAsset.h"
#include "../Data/CombatRulesDataAsset.h"
#include "../Data/StationIntegrityDataAsset.h"
#include "DamageCalculator.h"
#include "HealthComponent.h"
#include "VulnerabilityComponent.h"
#include "StationIntegrityComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "../Core/AtlasGameState.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentActionData = nullptr;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    
    DamageCalculator = NewObject<UDamageCalculator>(this);
    
    // Cache component references for performance
    HealthComponent = GetOwner()->FindComponentByClass<UHealthComponent>();
    ensure(HealthComponent); // Warn if missing but don't crash
    
    VulnerabilityComponent = GetOwner()->FindComponentByClass<UVulnerabilityComponent>();
    if (!VulnerabilityComponent)
    {
        VulnerabilityComponent = NewObject<UVulnerabilityComponent>(GetOwner(), UVulnerabilityComponent::StaticClass(), TEXT("VulnerabilityComponent"));
        VulnerabilityComponent->RegisterComponent();
    }
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCombatComponent::StartAttack(const FGameplayTag& AttackTag)
{
    if ((HealthComponent && HealthComponent->IsStaggered()) || IsAttacking())
    {
        return false;
    }

    UAttackDataAsset** AttackDataPtr = AttackDataMap.Find(AttackTag);
    if (!AttackDataPtr || !(*AttackDataPtr))
    {
        UE_LOG(LogTemp, Error, TEXT("Attack failed: No AttackData found for tag %s. Make sure AttackDataMap is configured in Blueprint!"), 
            *AttackTag.ToString());
        return false;
    }

    CurrentAttackData = *AttackDataPtr;
    
    // Apply station integrity cost for high-risk abilities (can destroy station!)
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(GetWorld());
    if (GameState && GameState->StationIntegrityComponent)
    {
        UStationIntegrityComponent* IntegrityComp = GameState->StationIntegrityComponent;
        if (IntegrityComp->IntegrityDataAsset)
        {
            float IntegrityCost = IntegrityComp->IntegrityDataAsset->GetIntegrityCostForAbility(AttackTag);
            if (IntegrityCost > 0.0f)
            {
                // Apply the integrity cost regardless - player must manage the risk!
                IntegrityComp->ApplyAbilityIntegrityCost(AttackTag, GetOwner());
                UE_LOG(LogTemp, Warning, TEXT("Applied %f integrity damage for ability %s - Station at %f%%"), 
                    IntegrityCost, *AttackTag.ToString(), IntegrityComp->GetIntegrityPercent());
                
                if (IntegrityComp->GetIntegrityPercent() <= 50.0f)
                {
                    UE_LOG(LogTemp, Error, TEXT("WARNING: Station integrity critical!"));
                }
            }
        }
    }
    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
    LastCombatActionTime = GetWorld()->GetTimeSeconds();


    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (CurrentAttackData->AttackData.AttackMontage)
        {
            Character->PlayAnimMontage(CurrentAttackData->AttackData.AttackMontage);
        }
    }

    OnAttackStarted.Broadcast(AttackTag, CurrentAttackData);
    
    // Auto-end attack after 1 second for testing (will be handled by animation notifies in Phase 2)
    FTimerHandle TempAttackTimer;
    GetWorld()->GetTimerManager().SetTimer(TempAttackTimer, this, &UCombatComponent::EndAttack, 1.0f, false);
    
    return true;
}

void UCombatComponent::EndAttack()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
    CurrentAttackData = nullptr;
    CurrentActionData = nullptr;  // Clear unified action data
    OnAttackEnded.Broadcast();
}

bool UCombatComponent::StartBlock()
{
    if ((HealthComponent && HealthComponent->IsStaggered()) || IsAttacking())
    {
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    LastCombatActionTime = GetWorld()->GetTimeSeconds();
    OnBlockStarted.Broadcast(true);
    return true;
}

void UCombatComponent::EndBlock()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockEnded.Broadcast();
}





void UCombatComponent::ProcessHit(AActor* HitActor, const FGameplayTag& AttackTag)
{
    if (!HitActor || !CurrentAttackData || !DamageCalculator)
    {
        return;
    }

    UCombatComponent* TargetCombat = HitActor->FindComponentByClass<UCombatComponent>();
    if (!TargetCombat)
    {
        return;
    }


    DamageCalculator->ProcessDamage(
        GetOwner(),
        HitActor,
        CurrentAttackData,
        CombatStateTags,
        TargetCombat->CombatStateTags
    );

    if (UHealthComponent* TargetHealth = HitActor->FindComponentByClass<UHealthComponent>())
    {
        TargetHealth->TakePoiseDamage(CurrentAttackData->AttackData.StaggerDamage);
    }
}

void UCombatComponent::SetCurrentActionData(UActionDataAsset* ActionData)
{
    CurrentActionData = ActionData;
}

void UCombatComponent::ProcessHitFromAnimation(AGameCharacterBase* HitCharacter)
{
    // For unified action system, use CurrentActionData if available
    if (!HitCharacter || !DamageCalculator)
    {
        return;
    }
    
    // Check if we have action data from unified system
    if (CurrentActionData)
    {
        // Use ActionDataAsset for damage calculation
        if (UHealthComponent* TargetHealth = HitCharacter->FindComponentByClass<UHealthComponent>())
        {
            // Apply damage from ActionDataAsset
            float Damage = CurrentActionData->MeleeDamage;
            TargetHealth->TakeDamage(Damage, GetOwner());
            
            // Apply poise damage
            if (CurrentActionData->PoiseDamage > 0.0f)
            {
                TargetHealth->TakePoiseDamage(CurrentActionData->PoiseDamage);
            }
            
            // Apply knockback
            if (CurrentActionData->KnockbackForce > 0.0f)
            {
                FVector KnockbackDirection = (HitCharacter->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
                if (ACharacter* TargetCharacter = Cast<ACharacter>(HitCharacter))
                {
                    TargetCharacter->LaunchCharacter(KnockbackDirection * CurrentActionData->KnockbackForce, true, true);
                    
                    // Apply ragdoll if configured
                    if (CurrentActionData->bCausesRagdoll)
                    {
                        // Trigger ragdoll effect
                        TargetCharacter->GetMesh()->SetSimulatePhysics(true);
                        TargetCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Hit from animation with ActionData: %f damage"), Damage);
        }
        return;
    }
    
    // Fallback to old system if no ActionData
    if (!CurrentAttackData)
    {
        return;
    }

    UCombatComponent* TargetCombat = HitCharacter->FindComponentByClass<UCombatComponent>();
    if (!TargetCombat)
    {
        return;
    }


    DamageCalculator->ProcessDamage(
        GetOwner(),
        HitCharacter,
        CurrentAttackData,
        CombatStateTags,
        TargetCombat->CombatStateTags
    );

    if (UHealthComponent* TargetHealth = HitCharacter->FindComponentByClass<UHealthComponent>())
    {
        TargetHealth->TakePoiseDamage(CurrentAttackData->AttackData.StaggerDamage);
    }
}


bool UCombatComponent::IsAttacking() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
}

bool UCombatComponent::IsBlocking() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
}


bool UCombatComponent::IsVulnerable() const
{
    if (VulnerabilityComponent)
    {
        return VulnerabilityComponent->IsVulnerable();
    }
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
}

bool UCombatComponent::HasIFrames() const
{
    if (VulnerabilityComponent)
    {
        return VulnerabilityComponent->HasIFrames();
    }
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.IFrames")));
}

void UCombatComponent::ApplyVulnerabilityWithIFrames(int32 Charges, bool bGrantIFrames)
{
    if (VulnerabilityComponent)
    {
        float Duration = CombatRules ? CombatRules->CombatRules.VulnerabilityDuration : 1.0f;
        VulnerabilityComponent->ApplyVulnerability(Charges, Duration);
        
        if (bGrantIFrames && VulnerabilityComponent->bEnableIFrames)
        {
            VulnerabilityComponent->StartIFrames();
        }
        
        OnVulnerabilityApplied.Broadcast();
    }
}


bool UCombatComponent::IsInCombat() const
{
    // Check if we're actively attacking, blocking, or recently damaged
    if (IsAttacking() || IsBlocking() || (HealthComponent && HealthComponent->IsStaggered()))
    {
        return true;
    }
    
    // Check if we've been in combat within the last 3 seconds
    float TimeSinceLastAction = GetTimeSinceLastCombatAction();
    return TimeSinceLastAction < 3.0f;
}

float UCombatComponent::GetTimeSinceLastCombatAction() const
{
    if (LastCombatActionTime <= 0.0f)
    {
        return 999.0f; // Never been in combat
    }
    
    return GetWorld() ? GetWorld()->GetTimeSeconds() - LastCombatActionTime : 999.0f;
}

void UCombatComponent::AddCombatStateTag(const FGameplayTag& Tag)
{
    CombatStateTags.AddTag(Tag);
    UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Added tag %s (Total tags: %d)"), 
        *Tag.ToString(), CombatStateTags.Num());
}

void UCombatComponent::RemoveCombatStateTag(const FGameplayTag& Tag)
{
    CombatStateTags.RemoveTag(Tag);
    UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Removed tag %s (Total tags: %d)"), 
        *Tag.ToString(), CombatStateTags.Num());
}

bool UCombatComponent::HasCombatStateTag(const FGameplayTag& Tag) const
{
    return CombatStateTags.HasTag(Tag);
}



UAnimMontage* UCombatComponent::GetAttackMontage(const FGameplayTag& AttackTag) const
{
    if (AttackDataMap.Contains(AttackTag))
    {
        UAttackDataAsset* AttackData = AttackDataMap[AttackTag];
        if (AttackData)
        {
            return AttackData->AttackData.AttackMontage;
        }
    }
    return nullptr;
}