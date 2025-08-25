#include "AtlasConsoleCommands.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Components/ActionManagerComponent.h"
#include "Atlas/Data/ActionDataAsset.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/AtlasGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagContainer.h"

bool FAtlasConsoleCommands::bGodModeEnabled = false;

void FAtlasConsoleCommands::RegisterCommands()
{
    // Core Run Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.StartRun"),
        TEXT("Start a new run (teleports to first room and spawns enemy)"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::StartRun),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.CompleteRoom"),
        TEXT("Complete current room and advance to next"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::CompleteRoom),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SelectReward"),
        TEXT("Select reward by index (0 or 1). Usage: Atlas.SelectReward 0"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::SelectReward),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.CancelRewardSelection"),
        TEXT("Cancel reward selection (skip reward)"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::CancelRewardSelection),
        ECVF_Cheat
    );
    
    // Player State Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SetHealth"),
        TEXT("Set player health (0-100)"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::SetHealth),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SetIntegrity"),
        TEXT("Set station integrity (0-100)"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::SetIntegrity),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.GodMode"),
        TEXT("Toggle god mode"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::GodMode),
        ECVF_Cheat
    );
    
    // Action System Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.AssignAction"),
        TEXT("Assign action to slot. Usage: Atlas.AssignAction Slot1 Action.Combat.BasicAttack"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::AssignAction),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ShowSlots"),
        TEXT("Show current action slot assignments"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::ShowSlots),
        ECVF_Cheat
    );
    
    // Debug Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SpawnEnemy"),
        TEXT("Spawn enemy at player location"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::SpawnEnemy),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.KillEnemy"),
        TEXT("Kill current enemy"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::KillEnemy),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Debug"),
        TEXT("Show debug info about game state"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::ShowDebugInfo),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ShowRooms"),
        TEXT("Show all available rooms and their configuration"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FAtlasConsoleCommands::ShowRooms),
        ECVF_Cheat
    );
    
    UE_LOG(LogTemp, Log, TEXT("Atlas Console Commands Registered"));
}

void FAtlasConsoleCommands::StartRun(const TArray<FString>& Args)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                RunManager->StartNewRun();
                UE_LOG(LogTemp, Warning, TEXT("Started new run"));
            }
        }
    }
}

void FAtlasConsoleCommands::SetHealth(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.SetHealth <0-100>"));
        return;
    }
    
    float Health = FMath::Clamp(FCString::Atof(*Args[0]), 0.0f, 100.0f);
    
    if (AGameCharacterBase* Player = GetPlayerCharacter())
    {
        if (UHealthComponent* HealthComp = Player->GetHealthComponent())
        {
            // Set health by healing or damaging to reach target
            float CurrentHealth = HealthComp->GetCurrentHealth();
            float HealthDiff = Health - CurrentHealth;
            
            if (HealthDiff > 0)
            {
                HealthComp->Heal(HealthDiff, nullptr);
            }
            else if (HealthDiff < 0)
            {
                HealthComp->TakeDamage(-HealthDiff, nullptr);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Set health to %.1f"), Health);
        }
    }
}

void FAtlasConsoleCommands::SetIntegrity(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.SetIntegrity <0-100>"));
        return;
    }
    
    float Integrity = FMath::Clamp(FCString::Atof(*Args[0]), 0.0f, 100.0f);
    
    if (AGameCharacterBase* Player = GetPlayerCharacter())
    {
        if (UStationIntegrityComponent* IntegrityComp = Player->FindComponentByClass<UStationIntegrityComponent>())
        {
            IntegrityComp->SetIntegrityPercent(Integrity);
            UE_LOG(LogTemp, Warning, TEXT("Set integrity to %.1f%%"), Integrity);
        }
    }
}

void FAtlasConsoleCommands::SpawnEnemy(const TArray<FString>& Args)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AGameCharacterBase* Player = GetPlayerCharacter())
        {
            FVector SpawnLocation = Player->GetActorLocation() + FVector(300, 0, 0);
            
            // Try to spawn BP_Enemy
            UClass* EnemyClass = LoadClass<APawn>(nullptr, TEXT("/Game/Blueprints/BP_Enemy.BP_Enemy_C"));
            if (EnemyClass)
            {
                World->SpawnActor<APawn>(EnemyClass, SpawnLocation, FRotator::ZeroRotator);
                UE_LOG(LogTemp, Warning, TEXT("Spawned enemy"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Could not load BP_Enemy"));
            }
        }
    }
}

void FAtlasConsoleCommands::CompleteRoom(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Warning, TEXT("Atlas.CompleteRoom command executed"));
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                UE_LOG(LogTemp, Warning, TEXT("Found RunManager, calling CompleteRoomTest()"));
                // Call the test function that properly simulates enemy death and triggers reward selection
                RunManager->CompleteRoomTest();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("RunManager not found in GameMode"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AtlasGameMode not found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World not found"));
    }
}

void FAtlasConsoleCommands::AssignAction(const TArray<FString>& Args)
{
    if (Args.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.AssignAction <SlotName> <ActionTag>"));
        UE_LOG(LogTemp, Error, TEXT("Example: Atlas.AssignAction Slot1 Action.Combat.BasicAttack"));
        return;
    }
    
    if (AGameCharacterBase* Player = GetPlayerCharacter())
    {
        if (UActionManagerComponent* ActionManager = Player->GetActionManagerComponent())
        {
            FName SlotName = FName(*Args[0]);
            FGameplayTag ActionTag = FGameplayTag::RequestGameplayTag(FName(*Args[1]));
            
            if (ActionManager->AssignActionToSlot(SlotName, ActionTag))
            {
                UE_LOG(LogTemp, Warning, TEXT("Assigned %s to %s"), *Args[1], *Args[0]);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to assign action"));
            }
        }
    }
}

void FAtlasConsoleCommands::ShowSlots(const TArray<FString>& Args)
{
    if (AGameCharacterBase* Player = GetPlayerCharacter())
    {
        if (UActionManagerComponent* ActionManager = Player->GetActionManagerComponent())
        {
            ActionManager->ExecuteShowSlotsCommand();
        }
    }
}

void FAtlasConsoleCommands::KillEnemy(const TArray<FString>& Args)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        // Find all enemies in world
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AGameCharacterBase::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
            {
                // Skip player
                if (Character->IsPlayerControlled())
                    continue;
                
                // Kill enemy
                if (UHealthComponent* Health = Character->GetHealthComponent())
                {
                    Health->TakeDamage(99999.0f, nullptr);
                    UE_LOG(LogTemp, Warning, TEXT("Killed enemy: %s"), *Character->GetName());
                    return;
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("No enemy found to kill"));
    }
}

void FAtlasConsoleCommands::GodMode(const TArray<FString>& Args)
{
    bGodModeEnabled = !bGodModeEnabled;
    
    if (AGameCharacterBase* Player = GetPlayerCharacter())
    {
        if (UHealthComponent* HealthComp = Player->GetHealthComponent())
        {
            HealthComp->SetInvincible(bGodModeEnabled);
            UE_LOG(LogTemp, Warning, TEXT("God Mode: %s"), bGodModeEnabled ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

void FAtlasConsoleCommands::ShowDebugInfo(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ATLAS DEBUG INFO ==="));
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        // Check GameMode and RunManager
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            UE_LOG(LogTemp, Warning, TEXT("GameMode: Valid"));
            
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                UE_LOG(LogTemp, Warning, TEXT("RunManager: Valid"));
                UE_LOG(LogTemp, Warning, TEXT("  - Current Level: %d/5"), RunManager->GetCurrentLevel());
                UE_LOG(LogTemp, Warning, TEXT("  - Run Active: %s"), RunManager->IsRunActive() ? TEXT("YES") : TEXT("NO"));
                
                if (URoomDataAsset* CurrentRoom = RunManager->GetCurrentRoom())
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - Current Room: %s"), *CurrentRoom->RoomName.ToString());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - Current Room: None"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("RunManager: NOT FOUND"));
            }
        }
        
        // Check Player
        if (AGameCharacterBase* Player = GetPlayerCharacter())
        {
            UE_LOG(LogTemp, Warning, TEXT("Player: %s"), *Player->GetName());
            
            // Check ActionManager
            if (UActionManagerComponent* ActionManager = Player->GetActionManagerComponent())
            {
                UE_LOG(LogTemp, Warning, TEXT("ActionManager: Valid"));
                TArray<UActionDataAsset*> Actions = ActionManager->GetAvailableActionDataAssets();
                UE_LOG(LogTemp, Warning, TEXT("  - Available Actions: %d"), Actions.Num());
                
                for (UActionDataAsset* Action : Actions)
                {
                    if (Action)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("    • %s (%s)"), 
                            *Action->ActionName.ToString(), 
                            *Action->ActionTag.ToString());
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ActionManager: NOT FOUND"));
            }
            
            // Check Health
            if (UHealthComponent* Health = Player->GetHealthComponent())
            {
                UE_LOG(LogTemp, Warning, TEXT("Health: %.1f/%.1f"), 
                    Health->GetCurrentHealth(), 
                    Health->GetMaxHealth());
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Player: NOT FOUND"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void FAtlasConsoleCommands::ShowRooms(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ROOM CONFIGURATION ==="));
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                TArray<URoomDataAsset*> AllRooms = RunManager->GetAllRoomDataAssets();
                UE_LOG(LogTemp, Warning, TEXT("Total Rooms: %d"), AllRooms.Num());
                
                for (URoomDataAsset* Room : AllRooms)
                {
                    if (Room)
                    {
                        FString LevelsStr = TEXT("[");
                        for (int32 Level : Room->AppearOnLevels)
                        {
                            LevelsStr += FString::Printf(TEXT("%d,"), Level);
                        }
                        if (Room->AppearOnLevels.Num() > 0)
                        {
                            LevelsStr.RemoveAt(LevelsStr.Len() - 1); // Remove last comma
                        }
                        LevelsStr += TEXT("]");
                        
                        UE_LOG(LogTemp, Warning, TEXT("  • %s"), *Room->RoomName.ToString());
                        UE_LOG(LogTemp, Warning, TEXT("    - ID: %s"), *Room->RoomID.ToString());
                        UE_LOG(LogTemp, Warning, TEXT("    - Levels: %s"), *LevelsStr);
                        UE_LOG(LogTemp, Warning, TEXT("    - Enemy: %s"), 
                            Room->UniqueEnemy ? *Room->UniqueEnemy->GetName() : TEXT("None"));
                        UE_LOG(LogTemp, Warning, TEXT("    - Can Repeat: %s"), 
                            Room->bCanRepeat ? TEXT("Yes") : TEXT("No"));
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void FAtlasConsoleCommands::SelectReward(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: Atlas.SelectReward <0-1>"));
        UE_LOG(LogTemp, Error, TEXT("Example: Atlas.SelectReward 0 (first reward) or Atlas.SelectReward 1 (second reward)"));
        return;
    }
    
    int32 RewardIndex = FCString::Atoi(*Args[0]);
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                RunManager->SelectReward(RewardIndex);
            }
        }
    }
}

void FAtlasConsoleCommands::CancelRewardSelection(const TArray<FString>& Args)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
        {
            if (URunManagerComponent* RunManager = GameMode->GetRunManager())
            {
                RunManager->CancelRewardSelection();
            }
        }
    }
}

AGameCharacterBase* FAtlasConsoleCommands::GetPlayerCharacter()
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return Cast<AGameCharacterBase>(PC->GetPawn());
        }
    }
    return nullptr;
}