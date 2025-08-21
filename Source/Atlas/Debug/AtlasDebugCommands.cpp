#include "AtlasDebugCommands.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/VulnerabilityComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/FocusModeComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Core/AtlasGameState.h"

// Static member initialization
bool FAtlasDebugCommands::bGodModeEnabled = false;
bool FAtlasDebugCommands::bOneHitKillEnabled = false;
bool FAtlasDebugCommands::bInfinitePoiseEnabled = false;
bool FAtlasDebugCommands::bNoCooldownsEnabled = false;
bool FAtlasDebugCommands::bNoIntegrityCostEnabled = false;

void FAtlasDebugCommands::RegisterAllCommands()
{
    // ===== FOCUS MODE COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ToggleFocusMode"),
        TEXT("Toggle focus mode on/off"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ToggleFocusMode));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ShowFocusDebug"),
        TEXT("Show focus mode debug visualization"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ShowFocusDebug));
    
    // ===== VULNERABILITY COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ApplyStunned"),
        TEXT("Apply stunned vulnerability tier"),
        FConsoleCommandDelegate::CreateStatic(&FAtlasDebugCommands::ApplyStunnedTier));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ApplyCrippled"),
        TEXT("Apply crippled vulnerability tier"),
        FConsoleCommandDelegate::CreateStatic(&FAtlasDebugCommands::ApplyCrippledTier));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ApplyExposed"),
        TEXT("Apply exposed vulnerability tier"),
        FConsoleCommandDelegate::CreateStatic(&FAtlasDebugCommands::ApplyExposedTier));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ClearVulnerability"),
        TEXT("Clear all vulnerability"),
        FConsoleCommandDelegate::CreateStatic(&FAtlasDebugCommands::ClearVulnerability));
    
    // ===== STATION INTEGRITY COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.DamageIntegrity"),
        TEXT("Damage station integrity by amount"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::DamageIntegrity));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SetIntegrityPercent"),
        TEXT("Set station integrity to percent"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::SetIntegrityPercent));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ShowIntegrityStatus"),
        TEXT("Show current station integrity status"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ShowIntegrityStatus));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ResetIntegrity"),
        TEXT("Reset station integrity to max"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ResetIntegrity));
    
    // ===== COMBAT COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.GodMode"),
        TEXT("Toggle god mode"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::GodMode));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.OneHitKill"),
        TEXT("Toggle one hit kill mode"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::OneHitKill));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.InfinitePoise"),
        TEXT("Toggle infinite poise"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::InfinitePoise));
    
    // ===== ABILITY COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.NoCooldowns"),
        TEXT("Toggle no cooldowns for abilities"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::NoCooldowns));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.NoIntegrityCost"),
        TEXT("Toggle no integrity cost for abilities"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::NoIntegrityCost));
    
    // ===== ACTION MANAGER COMMANDS =====
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_AssignAction"),
        TEXT("Assign an action to a slot. Usage: Atlas_AssignAction <SlotName> <ActionTag>"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::AssignAction));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_ClearSlot"),
        TEXT("Clear a specific action slot. Usage: Atlas_ClearSlot <SlotName>"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ClearSlot));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_SwapSlots"),
        TEXT("Swap two action slots. Usage: Atlas_SwapSlots <Slot1> <Slot2>"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::SwapSlots));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_ListActions"),
        TEXT("List all available actions"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ListActions));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_ShowSlots"),
        TEXT("Show current slot assignments"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ShowSlots));
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas_ResetSlots"),
        TEXT("Reset all slots to default assignments"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasDebugCommands::ResetSlots));
    
    UE_LOG(LogTemp, Log, TEXT("Atlas Debug Commands Registered"));
}

// ===== FOCUS MODE IMPLEMENTATIONS =====
void FAtlasDebugCommands::ToggleFocusMode(const TArray<FString>& Args)
{
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        if (UFocusModeComponent* FocusComp = Character->FindComponentByClass<UFocusModeComponent>())
        {
            FocusComp->ToggleFocusMode();
            UE_LOG(LogTemp, Warning, TEXT("Focus Mode toggled"));
        }
    }
}

void FAtlasDebugCommands::ShowFocusDebug(const TArray<FString>& Args)
{
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        if (UFocusModeComponent* FocusComp = Character->FindComponentByClass<UFocusModeComponent>())
        {
            FocusComp->bDebugDrawFocusInfo = !FocusComp->bDebugDrawFocusInfo;
            UE_LOG(LogTemp, Warning, TEXT("Focus debug display: %s"), 
                FocusComp->bDebugDrawFocusInfo ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

// ===== VULNERABILITY IMPLEMENTATIONS =====
void FAtlasDebugCommands::ApplyStunnedTier()
{
    if (UVulnerabilityComponent* VulnComp = GetVulnerabilityComponent())
    {
        VulnComp->ApplyVulnerabilityTier(EVulnerabilityTier::Stunned);
        UE_LOG(LogTemp, Warning, TEXT("Applied Stunned vulnerability tier"));
    }
}

void FAtlasDebugCommands::ApplyCrippledTier()
{
    if (UVulnerabilityComponent* VulnComp = GetVulnerabilityComponent())
    {
        VulnComp->ApplyVulnerabilityTier(EVulnerabilityTier::Crippled);
        UE_LOG(LogTemp, Warning, TEXT("Applied Crippled vulnerability tier"));
    }
}

void FAtlasDebugCommands::ApplyExposedTier()
{
    if (UVulnerabilityComponent* VulnComp = GetVulnerabilityComponent())
    {
        VulnComp->ApplyVulnerabilityTier(EVulnerabilityTier::Exposed);
        UE_LOG(LogTemp, Warning, TEXT("Applied Exposed vulnerability tier"));
    }
}

void FAtlasDebugCommands::ClearVulnerability()
{
    if (UVulnerabilityComponent* VulnComp = GetVulnerabilityComponent())
    {
        VulnComp->EndVulnerability();
        UE_LOG(LogTemp, Warning, TEXT("Cleared vulnerability"));
    }
}

// ===== STATION INTEGRITY IMPLEMENTATIONS =====
void FAtlasDebugCommands::DamageIntegrity(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.DamageIntegrity <amount>"));
        return;
    }
    
    float Amount = FCString::Atof(*Args[0]);
    if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
    {
        IntegrityComp->ApplyIntegrityDamage(Amount, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Applied %f integrity damage. Current: %f/%f"), 
            Amount, IntegrityComp->GetCurrentIntegrity(), IntegrityComp->GetMaxIntegrity());
    }
}

void FAtlasDebugCommands::SetIntegrityPercent(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.SetIntegrityPercent <percent>"));
        return;
    }
    
    float Percent = FCString::Atof(*Args[0]);
    if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
    {
        IntegrityComp->SetIntegrityPercent(FMath::Clamp(Percent, 0.0f, 100.0f));
        UE_LOG(LogTemp, Warning, TEXT("Set integrity to %f%%"), Percent);
    }
}

void FAtlasDebugCommands::ShowIntegrityStatus(const TArray<FString>& Args)
{
    if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
    {
        float Current = IntegrityComp->GetCurrentIntegrity();
        float Max = IntegrityComp->GetMaxIntegrity();
        float Percent = IntegrityComp->GetIntegrityPercent();
        
        UE_LOG(LogTemp, Warning, TEXT("===== STATION INTEGRITY STATUS ====="));
        UE_LOG(LogTemp, Warning, TEXT("Current: %f / %f (%f%%)"), Current, Max, Percent);
        
        if (Percent <= 25.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("STATUS: CRITICAL"));
        }
        else if (Percent <= 50.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("STATUS: WARNING"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("STATUS: STABLE"));
        }
    }
}

void FAtlasDebugCommands::ResetIntegrity(const TArray<FString>& Args)
{
    if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
    {
        IntegrityComp->SetIntegrityPercent(100.0f);
        UE_LOG(LogTemp, Warning, TEXT("Station integrity reset to maximum"));
    }
}

// ===== COMBAT IMPLEMENTATIONS =====
void FAtlasDebugCommands::GodMode(const TArray<FString>& Args)
{
    bGodModeEnabled = !bGodModeEnabled;
    
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        if (UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>())
        {
            HealthComp->SetInvincible(bGodModeEnabled);
            UE_LOG(LogTemp, Warning, TEXT("God Mode: %s"), bGodModeEnabled ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

void FAtlasDebugCommands::OneHitKill(const TArray<FString>& Args)
{
    bOneHitKillEnabled = !bOneHitKillEnabled;
    UE_LOG(LogTemp, Warning, TEXT("One Hit Kill: %s"), bOneHitKillEnabled ? TEXT("ON") : TEXT("OFF"));
    // Implementation would need to be hooked into damage calculation
}

void FAtlasDebugCommands::InfinitePoise(const TArray<FString>& Args)
{
    bInfinitePoiseEnabled = !bInfinitePoiseEnabled;
    
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        if (UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>())
        {
            if (bInfinitePoiseEnabled)
            {
                HealthComp->ResetPoise();
            }
            UE_LOG(LogTemp, Warning, TEXT("Infinite Poise: %s"), bInfinitePoiseEnabled ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

// ===== ABILITY IMPLEMENTATIONS =====
void FAtlasDebugCommands::NoCooldowns(const TArray<FString>& Args)
{
    bNoCooldownsEnabled = !bNoCooldownsEnabled;
    UE_LOG(LogTemp, Warning, TEXT("No Cooldowns: %s"), bNoCooldownsEnabled ? TEXT("ON") : TEXT("OFF"));
    // Implementation would need to be hooked into action system
}

void FAtlasDebugCommands::NoIntegrityCost(const TArray<FString>& Args)
{
    bNoIntegrityCostEnabled = !bNoIntegrityCostEnabled;
    UE_LOG(LogTemp, Warning, TEXT("No Integrity Cost: %s"), bNoIntegrityCostEnabled ? TEXT("ON") : TEXT("OFF"));
    // Implementation would need to be hooked into action system
}

// ===== HELPER IMPLEMENTATIONS =====
AGameCharacterBase* FAtlasDebugCommands::GetTargetCharacter()
{
    if (GEngine && GEngine->GameViewport)
    {
        if (UWorld* World = GEngine->GameViewport->GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                return Cast<AGameCharacterBase>(PC->GetPawn());
            }
        }
    }
    return nullptr;
}

UVulnerabilityComponent* FAtlasDebugCommands::GetVulnerabilityComponent()
{
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        return Character->FindComponentByClass<UVulnerabilityComponent>();
    }
    return nullptr;
}

UStationIntegrityComponent* FAtlasDebugCommands::GetStationIntegrity()
{
    if (GEngine && GEngine->GameViewport)
    {
        if (UWorld* World = GEngine->GameViewport->GetWorld())
        {
            if (AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World))
            {
                return GameState->StationIntegrityComponent;
            }
        }
    }
    return nullptr;
}

UActionManagerComponent* FAtlasDebugCommands::GetActionManager()
{
    if (AGameCharacterBase* Character = GetTargetCharacter())
    {
        return Character->FindComponentByClass<UActionManagerComponent>();
    }
    return nullptr;
}

// Placeholder implementations for commands not yet implemented
void FAtlasDebugCommands::SpawnTestInteractable(const TArray<FString>& Args) {}
void FAtlasDebugCommands::TestInteraction(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SetFocusRange(const TArray<FString>& Args) {}
void FAtlasDebugCommands::ForceUnlockFocus(const TArray<FString>& Args) {}
void FAtlasDebugCommands::ShowFocusInfo(const TArray<FString>& Args) {}
void FAtlasDebugCommands::TestSoftLock(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SimulateVentInteraction(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SimulateValveInteraction(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SimulateCriticalHit() {}
void FAtlasDebugCommands::ShowVulnerabilityInfo() {}
void FAtlasDebugCommands::ToggleDebugIndicators() {}
void FAtlasDebugCommands::SetStunnedMultiplier(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SetCrippledMultiplier(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SetExposedMultiplier(const TArray<FString>& Args) {}
void FAtlasDebugCommands::SetIntegrity(const TArray<FString>& Args) {}
void FAtlasDebugCommands::TestHighRiskAbility(const TArray<FString>& Args) {}
void FAtlasDebugCommands::ForceStagger(const TArray<FString>& Args) {}
void FAtlasDebugCommands::ResetCombat(const TArray<FString>& Args) {}
void FAtlasDebugCommands::UnlockAllAbilities(const TArray<FString>& Args) {}
void FAtlasDebugCommands::TestAbility(const TArray<FString>& Args) {}

// ===== ACTION MANAGER IMPLEMENTATIONS =====
void FAtlasDebugCommands::AssignAction(const TArray<FString>& Args)
{
    if (Args.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas_AssignAction <SlotName> <ActionTag>"));
        return;
    }
    
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        ActionManager->ExecuteAssignCommand(Args[0], Args[1]);
    }
}

void FAtlasDebugCommands::ClearSlot(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas_ClearSlot <SlotName>"));
        return;
    }
    
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        ActionManager->ExecuteClearSlotCommand(Args[0]);
    }
}

void FAtlasDebugCommands::SwapSlots(const TArray<FString>& Args)
{
    if (Args.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas_SwapSlots <Slot1> <Slot2>"));
        return;
    }
    
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        ActionManager->ExecuteSwapSlotsCommand(Args[0], Args[1]);
    }
}

void FAtlasDebugCommands::ListActions(const TArray<FString>& Args)
{
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        ActionManager->ExecuteListActionsCommand();
    }
}

void FAtlasDebugCommands::ShowSlots(const TArray<FString>& Args)
{
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        ActionManager->ExecuteShowSlotsCommand();
    }
}

void FAtlasDebugCommands::ResetSlots(const TArray<FString>& Args)
{
    if (UActionManagerComponent* ActionManager = GetActionManager())
    {
        // Clear all slots first
        ActionManager->ExecuteClearSlotCommand("Slot1");
        ActionManager->ExecuteClearSlotCommand("Slot2");
        ActionManager->ExecuteClearSlotCommand("Slot3");
        ActionManager->ExecuteClearSlotCommand("Slot4");
        ActionManager->ExecuteClearSlotCommand("Slot5");
        
        // Assign defaults
        ActionManager->ExecuteAssignCommand("Slot1", "Action.Ability.BasicAttack");
        ActionManager->ExecuteAssignCommand("Slot2", "Action.Ability.HeavyAttack");
        ActionManager->ExecuteAssignCommand("Slot3", "Action.Ability.Block");
        ActionManager->ExecuteAssignCommand("Slot4", "Action.Ability.Dash");
        ActionManager->ExecuteAssignCommand("Slot5", "Action.Ability.FocusMode");
        
        UE_LOG(LogTemp, Warning, TEXT("Reset all slots to default assignments"));
    }
}