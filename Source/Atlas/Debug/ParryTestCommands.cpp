#include "ParryTestCommands.h"
#include "../Components/CombatComponent.h"
#include "../Components/VulnerabilityComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ParryFeedbackComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UParryTestCommands::TestApplyVulnerability(int32 Charges)
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("No player character found"));
        return;
    }

    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    if (VulnComp)
    {
        VulnComp->ApplyVulnerability(Charges);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("Applied %d vulnerability charges (8x damage!)"), Charges));
        }
    }
    else
    {
        UCombatComponent* CombatComp = PlayerCharacter->FindComponentByClass<UCombatComponent>();
        if (CombatComp)
        {
            CombatComp->ApplyVulnerability(Charges);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                    FString::Printf(TEXT("Applied %d vulnerability charges via CombatComponent"), Charges));
            }
        }
    }
}

void UParryTestCommands::TestParryWindow()
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UCombatComponent* CombatComp = PlayerCharacter->FindComponentByClass<UCombatComponent>();
    if (CombatComp)
    {
        CombatComp->TryParry();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                TEXT("Parry window opened! Counter now!"));
        }
    }
}

void UParryTestCommands::TestIFrames(float Duration)
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    if (VulnComp)
    {
        VulnComp->StartIFrames(Duration);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("I-Frames active for %.2f seconds!"), Duration));
        }
    }
}

void UParryTestCommands::TestParrySuccess()
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UCombatComponent* CombatComp = PlayerCharacter->FindComponentByClass<UCombatComponent>();
    if (CombatComp)
    {
        CombatComp->HandleSuccessfulParry(nullptr);
        
        UParryFeedbackComponent* FeedbackComp = PlayerCharacter->FindComponentByClass<UParryFeedbackComponent>();
        if (FeedbackComp)
        {
            FeedbackComp->PlayParrySuccessFeedback(nullptr);
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("Simulated successful parry!"));
        }
    }
}

void UParryTestCommands::ShowParryInfo()
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UCombatComponent* CombatComp = PlayerCharacter->FindComponentByClass<UCombatComponent>();
    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    
    if (GEngine)
    {
        if (CombatComp)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
                FString::Printf(TEXT("=== Combat State ===")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
                FString::Printf(TEXT("Attacking: %s"), CombatComp->IsAttacking() ? TEXT("Yes") : TEXT("No")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
                FString::Printf(TEXT("Blocking: %s"), CombatComp->IsBlocking() ? TEXT("Yes") : TEXT("No")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
                FString::Printf(TEXT("Parrying: %s"), CombatComp->IsParrying() ? TEXT("Yes") : TEXT("No")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
                FString::Printf(TEXT("Has I-Frames: %s"), CombatComp->HasIFrames() ? TEXT("Yes") : TEXT("No")));
        }
        
        if (VulnComp)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("=== Vulnerability State ===")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Vulnerable: %s"), VulnComp->IsVulnerable() ? TEXT("Yes") : TEXT("No")));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Charges: %d"), VulnComp->GetRemainingCharges()));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Time Remaining: %.2fs"), VulnComp->GetTimeRemaining()));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Damage Multiplier: %.1fx"), VulnComp->GetDamageMultiplier()));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("I-Frames Enabled: %s"), VulnComp->bEnableIFrames ? TEXT("Yes") : TEXT("No")));
        }
    }
}

void UParryTestCommands::SetVulnerabilityMultiplier(float Multiplier)
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    if (VulnComp)
    {
        VulnComp->VulnerabilityDamageMultiplier = Multiplier;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                FString::Printf(TEXT("Vulnerability multiplier set to %.1fx"), Multiplier));
        }
    }
}

void UParryTestCommands::EnableIFrames(bool bEnable)
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    if (VulnComp)
    {
        VulnComp->bEnableIFrames = bEnable;
        VulnComp->bIFramesOnParrySuccess = bEnable;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                FString::Printf(TEXT("I-Frames %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled")));
        }
    }
}

void UParryTestCommands::TestDamageWithVulnerability(float BaseDamage)
{
    ACharacter* PlayerCharacter = GetOuterAPlayerController()->GetCharacter();
    if (!PlayerCharacter)
    {
        return;
    }

    UHealthComponent* HealthComp = PlayerCharacter->FindComponentByClass<UHealthComponent>();
    UVulnerabilityComponent* VulnComp = PlayerCharacter->FindComponentByClass<UVulnerabilityComponent>();
    
    if (HealthComp && VulnComp)
    {
        float Multiplier = VulnComp->GetDamageMultiplier();
        float FinalDamage = BaseDamage * Multiplier;
        
        HealthComp->TakeDamage(FinalDamage, nullptr);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("Test Damage: %.1f base * %.1fx = %.1f final"), 
                    BaseDamage, Multiplier, FinalDamage));
        }
        
        if (VulnComp->IsVulnerable())
        {
            VulnComp->ConsumeCharge();
        }
    }
}