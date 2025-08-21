#include "Phase3ConsoleCommands.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Components/RewardSelectionComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Core/AtlasGameMode.h"

#define PHASE3_LOG(Message) LogPhase3(Message, false)
#define PHASE3_ERROR(Message) LogPhase3(Message, true)

// ========================================
// REGISTRATION
// ========================================

void UPhase3ConsoleCommands::RegisterCommands()
{
	// Reward System Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ListRewards"),
		TEXT("List all available rewards"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ListAllRewards),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.GiveReward"),
		TEXT("Give player a specific reward. Usage: GiveReward [RewardName]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::GiveReward),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.PresentChoice"),
		TEXT("Present reward selection UI. Usage: PresentChoice [Reward1] [Reward2]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::PresentRewardChoice),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.EquipToSlot"),
		TEXT("Equip reward to slot. Usage: EquipToSlot [RewardName] [SlotIndex]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::EquipRewardToSlot),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ShowSlots"),
		TEXT("Show current slot status"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ShowSlots),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ClearSlot"),
		TEXT("Clear specific slot. Usage: ClearSlot [SlotIndex]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ClearSlot),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ClearAllSlots"),
		TEXT("Clear all reward slots"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ClearAllSlots),
		ECVF_Cheat
	);
	
	// Room System Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ListRooms"),
		TEXT("List all available rooms"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ListRooms),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.StartRun"),
		TEXT("Start a new run"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::StartRun),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.CompleteRoom"),
		TEXT("Complete current room"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::CompleteRoom),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ShowRunProgress"),
		TEXT("Show current run progress"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ShowRunProgress),
		ECVF_Cheat
	);
	
	// Enemy AI Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.SpawnEnemy"),
		TEXT("Spawn specific enemy. Usage: SpawnEnemy [EnemyType]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::SpawnEnemy),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.SetEnemyDifficulty"),
		TEXT("Set enemy difficulty. Usage: SetEnemyDifficulty [1-10]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::SetEnemyDifficulty),
		ECVF_Cheat
	);
	
	// UI Testing Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ShowRewardUI"),
		TEXT("Show reward selection UI for testing"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ShowRewardSelectionUI),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ShowSlotUI"),
		TEXT("Show slot manager UI for testing"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ShowSlotManagerUI),
		ECVF_Cheat
	);
	
	// Debug Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.Debug"),
		TEXT("Enable Phase 3 debug mode"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::EnablePhase3Debug),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.RunTests"),
		TEXT("Run Phase 3 validation tests"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::RunPhase3Tests),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.QuickTest"),
		TEXT("Quick test Phase 3 feature. Usage: QuickTest [Feature]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::QuickTest),
		ECVF_Cheat
	);
	
	PHASE3_LOG("Phase 3 Console Commands Registered");
}

void UPhase3ConsoleCommands::UnregisterCommands()
{
	// Commands are automatically unregistered when console manager is destroyed
	PHASE3_LOG("Phase 3 Console Commands Unregistered");
}

// ========================================
// REWARD SYSTEM COMMANDS
// ========================================

void UPhase3ConsoleCommands::ListAllRewards(const TArray<FString>& Args)
{
	PHASE3_LOG("=== PHASE 3 REWARD LIST ===");
	PHASE3_LOG("Defense Rewards:");
	PHASE3_LOG("  - ImprovedBlock (1 slot, 3 stacks)");
	PHASE3_LOG("  - ParryMaster (1 slot, 2 stacks)");
	PHASE3_LOG("  - CounterStrike (2 slots, 1 stack)");
	PHASE3_LOG("  - IronSkin (1 slot, 3 stacks)");
	PHASE3_LOG("  - LastStand (2 slots, 1 stack)");
	
	PHASE3_LOG("Offense Rewards:");
	PHASE3_LOG("  - SharpBlade (1 slot, 3 stacks)");
	PHASE3_LOG("  - HeavyImpact (2 slots, 2 stacks)");
	PHASE3_LOG("  - BleedingStrikes (1 slot, 3 stacks)");
	PHASE3_LOG("  - Executioner (2 slots, 1 stack)");
	PHASE3_LOG("  - RapidStrikes (1 slot, 3 stacks)");
	
	PHASE3_LOG("Passive Stats:");
	PHASE3_LOG("  - Vitality (1 slot, 3 stacks)");
	PHASE3_LOG("  - Swiftness (1 slot, 2 stacks)");
	PHASE3_LOG("  - Heavyweight (1 slot, 2 stacks)");
	PHASE3_LOG("  - Regeneration (2 slots, 2 stacks)");
	PHASE3_LOG("  - Fortitude (1 slot, 3 stacks)");
	
	PHASE3_LOG("Passive Abilities:");
	PHASE3_LOG("  - SecondWind (3 slots, 1 stack)");
	PHASE3_LOG("  - Vampirism (2 slots, 2 stacks)");
	PHASE3_LOG("  - Berserker (2 slots, 1 stack)");
	PHASE3_LOG("  - Momentum (2 slots, 1 stack)");
	PHASE3_LOG("  - StationShield (2 slots, 1 stack)");
	
	PHASE3_LOG("Interactables:");
	PHASE3_LOG("  - ExplosiveValves (1 slot, 2 stacks)");
	PHASE3_LOG("  - GravityWells (2 slots, 1 stack)");
	PHASE3_LOG("  - TurretHack (2 slots, 1 stack)");
	PHASE3_LOG("  - EmergencyVent (1 slot, 2 stacks)");
	PHASE3_LOG("  - PowerSurge (1 slot, 3 stacks)");
}

void UPhase3ConsoleCommands::GiveReward(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: Atlas.Phase3.GiveReward [RewardName]");
		return;
	}
	
	USlotManagerComponent* SlotManager = GetSlotManager();
	if (!SlotManager)
	{
		PHASE3_ERROR("SlotManager not found");
		return;
	}
	
	URewardDataAsset* Reward = FindReward(Args[0]);
	if (!Reward)
	{
		PHASE3_ERROR(FString::Printf(TEXT("Reward '%s' not found"), *Args[0]));
		return;
	}
	
	// Find first available slot
	int32 AvailableSlot = -1;
	for (int32 i = 0; i < 6; ++i)
	{
		if (!SlotManager->IsSlotOccupied(i))
		{
			AvailableSlot = i;
			break;
		}
	}
	
	if (AvailableSlot == -1)
	{
		PHASE3_ERROR("No available slots");
		return;
	}
	
	if (SlotManager->EquipReward(Reward, AvailableSlot))
	{
		PHASE3_LOG(FString::Printf(TEXT("Equipped '%s' to slot %d"), 
			*Reward->RewardName.ToString(), AvailableSlot));
	}
	else
	{
		PHASE3_ERROR("Failed to equip reward");
	}
}

void UPhase3ConsoleCommands::ShowSlots(const TArray<FString>& Args)
{
	USlotManagerComponent* SlotManager = GetSlotManager();
	if (!SlotManager)
	{
		PHASE3_ERROR("SlotManager not found");
		return;
	}
	
	PHASE3_LOG("=== CURRENT SLOT STATUS ===");
	
	for (int32 i = 0; i < 6; ++i)
	{
		if (SlotManager->IsSlotOccupied(i))
		{
			URewardDataAsset* Reward = SlotManager->GetRewardInSlot(i);
			int32 StackLevel = SlotManager->GetRewardStackLevel(i);
			
			if (Reward)
			{
				FString SlotInfo = FString::Printf(TEXT("Slot %d: %s (Level %d/%d)"),
					i,
					*Reward->RewardName.ToString(),
					StackLevel,
					Reward->MaxStackLevel
				);
				PHASE3_LOG(SlotInfo);
			}
		}
		else
		{
			PHASE3_LOG(FString::Printf(TEXT("Slot %d: [EMPTY]"), i));
		}
	}
	
	// Show total stats
	PHASE3_LOG("=== TOTAL STAT MODIFIERS ===");
	PHASE3_LOG(FString::Printf(TEXT("Max Health: +%.0f"), 
		SlotManager->CalculateTotalStatModifier("MaxHealth")));
	PHASE3_LOG(FString::Printf(TEXT("Move Speed: +%.0f%%"), 
		SlotManager->CalculateTotalStatModifier("MoveSpeed") * 100.0f));
	PHASE3_LOG(FString::Printf(TEXT("Damage Multiplier: +%.0f%%"), 
		SlotManager->CalculateTotalStatModifier("DamageMultiplier") * 100.0f));
	PHASE3_LOG(FString::Printf(TEXT("Damage Reduction: %.0f%%"), 
		SlotManager->CalculateTotalStatModifier("DamageReduction") * 100.0f));
}

// ========================================
// ROOM SYSTEM COMMANDS
// ========================================

void UPhase3ConsoleCommands::ListRooms(const TArray<FString>& Args)
{
	PHASE3_LOG("=== PHASE 3 ROOM LIST ===");
	PHASE3_LOG("Room A - Engineering Bay (Defense theme, Medium, Levels 1-3)");
	PHASE3_LOG("  Enemy: Chief Engineer Hayes (Defensive AI)");
	PHASE3_LOG("  Hazard: Electrical Surges");
	
	PHASE3_LOG("Room B - Medical Ward (Passive Stats theme, Easy, Levels 1-2)");
	PHASE3_LOG("  Enemy: Dr. Voss (Balanced AI)");
	PHASE3_LOG("  Hazard: Toxic Leak");
	
	PHASE3_LOG("Room C - Weapons Lab (Offense theme, Hard, Levels 2-4)");
	PHASE3_LOG("  Enemy: Commander Rex (Aggressive AI)");
	PHASE3_LOG("  Hazard: None");
	
	PHASE3_LOG("Room D - Command Center (Passive Ability theme, Hard, Levels 3-5)");
	PHASE3_LOG("  Enemy: Admiral Kronos (Tactical AI)");
	PHASE3_LOG("  Hazard: System Malfunction");
	
	PHASE3_LOG("Room E - Maintenance Shaft (Interactable theme, Medium, All Levels)");
	PHASE3_LOG("  Enemy: Unit M-471 (Tactical AI)");
	PHASE3_LOG("  Hazard: Low Gravity");
}

void UPhase3ConsoleCommands::StartRun(const TArray<FString>& Args)
{
	URunManagerComponent* RunManager = GetRunManager();
	if (!RunManager)
	{
		PHASE3_ERROR("RunManager not found");
		return;
	}
	
	RunManager->StartNewRun();
	PHASE3_LOG("New run started!");
	
	// Show initial room
	if (URoomDataAsset* CurrentRoom = RunManager->GetCurrentRoom())
	{
		PHASE3_LOG(FString::Printf(TEXT("Starting in: %s"), 
			*CurrentRoom->RoomName.ToString()));
		PHASE3_LOG(FString::Printf(TEXT("Enemy: %s"), 
			*CurrentRoom->EnemyName.ToString()));
	}
}

void UPhase3ConsoleCommands::ShowRunProgress(const TArray<FString>& Args)
{
	URunManagerComponent* RunManager = GetRunManager();
	if (!RunManager)
	{
		PHASE3_ERROR("RunManager not found");
		return;
	}
	
	PHASE3_LOG("=== RUN PROGRESS ===");
	PHASE3_LOG(FString::Printf(TEXT("Current Level: %d/5"), RunManager->GetCurrentLevel()));
	PHASE3_LOG(FString::Printf(TEXT("Rooms Completed: %d"), RunManager->GetRoomsCompleted()));
	
	if (URoomDataAsset* CurrentRoom = RunManager->GetCurrentRoom())
	{
		PHASE3_LOG(FString::Printf(TEXT("Current Room: %s"), 
			*CurrentRoom->RoomName.ToString()));
		PHASE3_LOG(FString::Printf(TEXT("Room Theme: %s"), 
			*UEnum::GetValueAsString(CurrentRoom->RoomTheme)));
		PHASE3_LOG(FString::Printf(TEXT("Difficulty: %s"), 
			*UEnum::GetValueAsString(CurrentRoom->Difficulty)));
	}
	
	if (RunManager->IsRunComplete())
	{
		PHASE3_LOG("Run Status: COMPLETE");
	}
	else if (RunManager->IsRunActive())
	{
		PHASE3_LOG("Run Status: ACTIVE");
	}
	else
	{
		PHASE3_LOG("Run Status: NOT STARTED");
	}
}

// ========================================
// DEBUG COMMANDS
// ========================================

void UPhase3ConsoleCommands::RunPhase3Tests(const TArray<FString>& Args)
{
	PHASE3_LOG("=== RUNNING PHASE 3 VALIDATION TESTS ===");
	
	int32 TestsPassed = 0;
	int32 TestsFailed = 0;
	
	// Test 1: Verify all reward DataAssets exist
	PHASE3_LOG("Test 1: Checking reward DataAssets...");
	TArray<FString> RewardNames = {
		"ImprovedBlock", "ParryMaster", "CounterStrike", "IronSkin", "LastStand",
		"SharpBlade", "HeavyImpact", "BleedingStrikes", "Executioner", "RapidStrikes",
		"Vitality", "Swiftness", "Heavyweight", "Regeneration", "Fortitude",
		"SecondWind", "Vampirism", "Berserker", "Momentum", "StationShield",
		"ExplosiveValves", "GravityWells", "TurretHack", "EmergencyVent", "PowerSurge"
	};
	
	for (const FString& RewardName : RewardNames)
	{
		if (FindReward(RewardName))
		{
			TestsPassed++;
		}
		else
		{
			PHASE3_ERROR(FString::Printf(TEXT("  Missing reward: %s"), *RewardName));
			TestsFailed++;
		}
	}
	
	// Test 2: Verify slot manager functionality
	PHASE3_LOG("Test 2: Testing slot manager...");
	if (USlotManagerComponent* SlotManager = GetSlotManager())
	{
		TestsPassed++;
		PHASE3_LOG("  SlotManager found and active");
	}
	else
	{
		TestsFailed++;
		PHASE3_ERROR("  SlotManager not found");
	}
	
	// Test 3: Verify run manager functionality
	PHASE3_LOG("Test 3: Testing run manager...");
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		TestsPassed++;
		PHASE3_LOG("  RunManager found and active");
	}
	else
	{
		TestsFailed++;
		PHASE3_ERROR("  RunManager not found");
	}
	
	// Test 4: Verify room DataAssets
	PHASE3_LOG("Test 4: Checking room DataAssets...");
	TArray<FString> RoomIDs = {
		"Room_A_Engineering", "Room_B_Medical", "Room_C_Weapons",
		"Room_D_Command", "Room_E_Maintenance"
	};
	
	for (const FString& RoomID : RoomIDs)
	{
		if (FindRoom(RoomID))
		{
			TestsPassed++;
		}
		else
		{
			PHASE3_ERROR(FString::Printf(TEXT("  Missing room: %s"), *RoomID));
			TestsFailed++;
		}
	}
	
	// Results
	PHASE3_LOG("=== TEST RESULTS ===");
	PHASE3_LOG(FString::Printf(TEXT("Tests Passed: %d"), TestsPassed));
	PHASE3_LOG(FString::Printf(TEXT("Tests Failed: %d"), TestsFailed));
	
	if (TestsFailed == 0)
	{
		PHASE3_LOG("✓ All Phase 3 tests passed!");
	}
	else
	{
		PHASE3_ERROR("✗ Some tests failed. Check implementation.");
	}
}

void UPhase3ConsoleCommands::QuickTest(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: Atlas.Phase3.QuickTest [rewards|rooms|ui|run]");
		return;
	}
	
	FString TestType = Args[0].ToLower();
	
	if (TestType == "rewards")
	{
		PHASE3_LOG("Quick testing reward system...");
		
		// Give player 3 random rewards
		TArray<FString> TestRewards = {"SharpBlade", "IronSkin", "Swiftness"};
		for (const FString& RewardName : TestRewards)
		{
			TArray<FString> GiveArgs = {RewardName};
			GiveReward(GiveArgs);
		}
		
		// Show slots
		ShowSlots(TArray<FString>());
	}
	else if (TestType == "rooms")
	{
		PHASE3_LOG("Quick testing room system...");
		
		// Start run and complete first room
		StartRun(TArray<FString>());
		CompleteRoom(TArray<FString>());
		ShowRunProgress(TArray<FString>());
	}
	else if (TestType == "ui")
	{
		PHASE3_LOG("Quick testing UI systems...");
		ShowRewardSelectionUI(TArray<FString>());
		ShowSlotManagerUI(TArray<FString>());
	}
	else if (TestType == "run")
	{
		PHASE3_LOG("Quick testing full run simulation...");
		SimulateRun(TArray<FString>());
	}
	else
	{
		PHASE3_ERROR("Unknown test type. Use: rewards, rooms, ui, or run");
	}
}

// ========================================
// HELPER FUNCTIONS
// ========================================

AGameCharacterBase* UPhase3ConsoleCommands::GetPlayerCharacter()
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return Cast<AGameCharacterBase>(PC->GetPawn());
		}
	}
	return nullptr;
}

USlotManagerComponent* UPhase3ConsoleCommands::GetSlotManager()
{
	if (AGameCharacterBase* Player = GetPlayerCharacter())
	{
		return Player->FindComponentByClass<USlotManagerComponent>();
	}
	return nullptr;
}

URunManagerComponent* UPhase3ConsoleCommands::GetRunManager()
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
		{
			return GameMode->FindComponentByClass<URunManagerComponent>();
		}
	}
	return nullptr;
}

URewardSelectionComponent* UPhase3ConsoleCommands::GetRewardSelection()
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		if (AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(World->GetAuthGameMode()))
		{
			return GameMode->FindComponentByClass<URewardSelectionComponent>();
		}
	}
	return nullptr;
}

URewardDataAsset* UPhase3ConsoleCommands::FindReward(const FString& Identifier)
{
	// In a real implementation, this would search through loaded DataAssets
	// For now, return nullptr as a placeholder
	return nullptr;
}

URoomDataAsset* UPhase3ConsoleCommands::FindRoom(const FString& RoomID)
{
	// In a real implementation, this would search through loaded DataAssets
	// For now, return nullptr as a placeholder
	return nullptr;
}

void UPhase3ConsoleCommands::LogPhase3(const FString& Message, bool bError)
{
	if (bError)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] %s"), *Message);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[PHASE3] %s"), *Message);
	}
	
	// Also print to screen for immediate feedback
	if (GEngine)
	{
		FColor Color = bError ? FColor::Red : FColor::Green;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, Color, FString::Printf(TEXT("[PHASE3] %s"), *Message));
	}
}