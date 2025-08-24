#include "Phase3ConsoleCommands.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Characters/EnemyCharacter.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Components/RunManagerSubsystem.h"
#include "Atlas/Components/RewardSelectionComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Interfaces/IHealthInterface.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Rooms/RoomBase.h"
#include "Atlas/AtlasGameMode.h"
#include "GlobalRunManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#define PHASE3_LOG(Message) LogPhase3(Message, false)
#define PHASE3_ERROR(Message) LogPhase3(Message, true)

// Static run tracking variables
TArray<ERoomType> UPhase3ConsoleCommands::CurrentRunOrder;
int32 UPhase3ConsoleCommands::CurrentRunIndex = 0;
AActor* UPhase3ConsoleCommands::CurrentSpawnedEnemy = nullptr;
FTimerHandle UPhase3ConsoleCommands::EnemyCheckTimer;

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
	
	// Room Teleport Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.GoToRoom"),
		TEXT("Teleport to specific room. Usage: GoToRoom [RoomName]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::GoToRoom),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.ResetRooms"),
		TEXT("Reset all rooms in test arena"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::ResetRooms),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.DebugRooms"),
		TEXT("Show debug info for all rooms"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::DebugRooms),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.StartTestRun"),
		TEXT("Start a complete 5-room run with randomized order"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::StartTestRun),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.NextRoom"),
		TEXT("Progress to next room in current run"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::NextRoom),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.CompleteAndProgress"),
		TEXT("Complete current room and auto-progress to next"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::CompleteAndProgress),
		ECVF_Cheat
	);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Atlas.Phase3.CheckSetup"),
		TEXT("Check if game setup is correct for testing"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase3ConsoleCommands::CheckSetup),
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
		if (!SlotManager->GetRewardInSlot(i))
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
		if (SlotManager->GetRewardInSlot(i))
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
	PHASE3_LOG(FString::Printf(TEXT("Rooms Completed: %d"), RunManager->GetCurrentLevel() - 1));
	
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
// ADDITIONAL COMMAND IMPLEMENTATIONS
// ========================================

void UPhase3ConsoleCommands::PresentRewardChoice(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		PHASE3_ERROR("Usage: PresentChoice [Reward1] [Reward2]");
		return;
	}
	
	URewardSelectionComponent* RewardSelection = GetRewardSelection();
	if (!RewardSelection)
	{
		PHASE3_ERROR("RewardSelectionComponent not found");
		return;
	}
	
	// Find the two rewards
	URewardDataAsset* Reward1 = FindReward(Args[0]);
	URewardDataAsset* Reward2 = FindReward(Args[1]);
	
	if (!Reward1 || !Reward2)
	{
		PHASE3_ERROR("Could not find specified rewards");
		return;
	}
	
	TArray<URewardDataAsset*> Choices;
	Choices.Add(Reward1);
	Choices.Add(Reward2);
	
	RewardSelection->PresentRewardChoice(Choices);
	PHASE3_LOG("Presented reward choice");
}

void UPhase3ConsoleCommands::EquipRewardToSlot(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		PHASE3_ERROR("Usage: EquipToSlot [RewardName] [SlotIndex]");
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
	
	int32 SlotIndex = FCString::Atoi(*Args[1]);
	if (SlotIndex < 0 || SlotIndex >= 6)
	{
		PHASE3_ERROR("Slot index must be between 0 and 5");
		return;
	}
	
	if (SlotManager->EquipReward(Reward, SlotIndex))
	{
		PHASE3_LOG(FString::Printf(TEXT("Equipped '%s' to slot %d"), 
			*Reward->RewardName.ToString(), SlotIndex));
	}
	else
	{
		PHASE3_ERROR("Failed to equip reward to slot");
	}
}

void UPhase3ConsoleCommands::ClearSlot(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: ClearSlot [SlotIndex]");
		return;
	}
	
	USlotManagerComponent* SlotManager = GetSlotManager();
	if (!SlotManager)
	{
		PHASE3_ERROR("SlotManager not found");
		return;
	}
	
	int32 SlotIndex = FCString::Atoi(*Args[0]);
	if (SlotIndex < 0 || SlotIndex >= 6)
	{
		PHASE3_ERROR("Slot index must be between 0 and 5");
		return;
	}
	
	if (SlotManager->RemoveReward(SlotIndex))
	{
		PHASE3_LOG(FString::Printf(TEXT("Cleared slot %d"), SlotIndex));
	}
	else
	{
		PHASE3_ERROR("Failed to clear slot");
	}
}

void UPhase3ConsoleCommands::ClearAllSlots(const TArray<FString>& Args)
{
	USlotManagerComponent* SlotManager = GetSlotManager();
	if (!SlotManager)
	{
		PHASE3_ERROR("SlotManager not found");
		return;
	}
	
	SlotManager->ClearAllRewards();
	PHASE3_LOG("Cleared all reward slots");
}

void UPhase3ConsoleCommands::CompleteRoom(const TArray<FString>& Args)
{
	URunManagerComponent* RunManager = GetRunManager();
	if (!RunManager)
	{
		PHASE3_ERROR("RunManager not found");
		return;
	}
	
	RunManager->CompleteCurrentRoom();
	
	if (RunManager->IsRunComplete())
	{
		PHASE3_LOG("Run completed!");
	}
	else
	{
		RunManager->TransitionToNextRoom();
		PHASE3_LOG(FString::Printf(TEXT("Advanced to room %d"), RunManager->GetCurrentLevel()));
	}
}

void UPhase3ConsoleCommands::SpawnEnemy(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: SpawnEnemy [EnemyType]");
		return;
	}
	
	// This would spawn an enemy in a real implementation
	PHASE3_LOG(FString::Printf(TEXT("Spawning enemy: %s"), *Args[0]));
	
	// Placeholder implementation
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		// Would spawn enemy blueprint here
		PHASE3_LOG("Enemy spawn system not yet implemented");
	}
}

void UPhase3ConsoleCommands::SetEnemyDifficulty(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: SetEnemyDifficulty [1-10]");
		return;
	}
	
	int32 Difficulty = FCString::Atoi(*Args[0]);
	if (Difficulty < 1 || Difficulty > 10)
	{
		PHASE3_ERROR("Difficulty must be between 1 and 10");
		return;
	}
	
	// This would set enemy difficulty in a real implementation
	PHASE3_LOG(FString::Printf(TEXT("Enemy difficulty set to %d"), Difficulty));
	
	// Find enemies in world and update their difficulty component
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		// Would update AIDifficultyComponent here
		PHASE3_LOG("Enemy difficulty system not yet implemented");
	}
}

void UPhase3ConsoleCommands::ShowRewardSelectionUI(const TArray<FString>& Args)
{
	PHASE3_LOG("Opening reward selection UI...");
	
	// This would show the reward selection UI in a real implementation
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			// Would create and show UI widget here
			PHASE3_LOG("Reward selection UI not yet implemented");
		}
	}
}

void UPhase3ConsoleCommands::ShowSlotManagerUI(const TArray<FString>& Args)
{
	PHASE3_LOG("Opening slot manager UI...");
	
	// This would show the slot manager UI in a real implementation
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			// Would create and show UI widget here
			PHASE3_LOG("Slot manager UI not yet implemented");
		}
	}
}

void UPhase3ConsoleCommands::EnablePhase3Debug(const TArray<FString>& Args)
{
	PHASE3_LOG("Phase 3 debug mode enabled");
	
	// Enable additional debug visualization
	if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
	{
		// Would enable debug drawing and logging here
		// In a real implementation, this would set a global debug flag or CVars
		PHASE3_LOG("Debug visualizations activated");
	}
}

void UPhase3ConsoleCommands::SimulateRun(const TArray<FString>& Args)
{
	PHASE3_LOG("=== SIMULATING FULL RUN ===");
	
	// Start run
	StartRun(TArray<FString>());
	
	// Simulate 5 rooms
	for (int32 i = 0; i < 5; ++i)
	{
		PHASE3_LOG(FString::Printf(TEXT("Room %d simulation..."), i + 1));
		
		// Simulate combat victory
		PHASE3_LOG("  - Enemy defeated");
		
		// Give random reward
		TArray<FString> RewardArgs;
		RewardArgs.Add("ImprovedBlock");
		GiveReward(RewardArgs);
		
		// Complete room
		CompleteRoom(TArray<FString>());
		
		if (i < 4)
		{
			PHASE3_LOG("  - Transitioning to next room");
		}
	}
	
	PHASE3_LOG("=== RUN SIMULATION COMPLETE ===");
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
	// Debug: Check basic engine state
	if (!GEngine)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] GEngine is null!"));
		return nullptr;
	}
	
	if (GEngine->GetWorldContexts().Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] No world contexts available!"));
		return nullptr;
	}
	
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] World is null!"));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[PHASE3] World found: %s"), *World->GetName());
	
	// Method 1: Try to get from GameMode
	AGameModeBase* GameModeBase = World->GetAuthGameMode();
	if (!GameModeBase)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] No GameMode found in world!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PHASE3] GameMode class: %s"), *GameModeBase->GetClass()->GetName());
		
		// First try casting to our C++ GameMode
		AAtlasGameMode* GameMode = Cast<AAtlasGameMode>(GameModeBase);
		if (GameMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PHASE3] Successfully cast to AAtlasGameMode"));
			URunManagerComponent* RunManager = GameMode->GetRunManager();
			if (RunManager)
			{
				UE_LOG(LogTemp, Log, TEXT("[PHASE3] Got RunManager from GameMode"));
				return RunManager;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[PHASE3] GameMode->GetRunManager() returned null"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PHASE3] Could not cast to AAtlasGameMode, trying FindComponentByClass"));
		}
		
		// If that didn't work, try finding the component on the GameMode actor
		URunManagerComponent* RunManager = GameModeBase->FindComponentByClass<URunManagerComponent>();
		if (RunManager)
		{
			UE_LOG(LogTemp, Log, TEXT("[PHASE3] Found RunManager component on GameMode actor"));
			return RunManager;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PHASE3] FindComponentByClass<URunManagerComponent>() returned null"));
		}
	}
	
	// Method 2: Try the subsystem approach
	UE_LOG(LogTemp, Warning, TEXT("[PHASE3] Trying subsystem approach..."));
	
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[PHASE3] GameInstance is null!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PHASE3] GameInstance found: %s"), *GameInstance->GetClass()->GetName());
		
		URunManagerSubsystem* Subsystem = GameInstance->GetSubsystem<URunManagerSubsystem>();
		if (!Subsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[PHASE3] RunManagerSubsystem not found! Subsystem might not be registered."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PHASE3] RunManagerSubsystem found, getting RunManager..."));
			URunManagerComponent* RunManager = Subsystem->GetRunManager();
			if (RunManager)
			{
				UE_LOG(LogTemp, Log, TEXT("[PHASE3] Got RunManager from Subsystem"));
				return RunManager;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[PHASE3] Subsystem->GetRunManager() returned null"));
			}
		}
	}
	
	// Method 3: Use global singleton as last resort
	UE_LOG(LogTemp, Warning, TEXT("[PHASE3] Trying global singleton approach..."));
	URunManagerComponent* GlobalRunManager = FGlobalRunManager::Get();
	if (GlobalRunManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PHASE3] Got RunManager from global singleton (fallback)"));
		return GlobalRunManager;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[PHASE3] RunManager not found - tried GameMode, Subsystem, and Global approaches"));
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

// ========================================
// ROOM TELEPORT COMMANDS IMPLEMENTATION
// ========================================

void UPhase3ConsoleCommands::GoToRoom(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PHASE3_ERROR("Usage: Atlas.Phase3.GoToRoom [RoomName]");
		PHASE3_LOG("Valid rooms: EngineeringBay, CombatArena, MedicalBay, CargoHold, Bridge");
		return;
	}
	
	FString RoomName = Args[0];
	
	// Get the RunManager
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->GoToRoom(RoomName);
		PHASE3_LOG(FString::Printf(TEXT("Attempting to teleport to room: %s"), *RoomName));
	}
	else
	{
		// Try to find room actors directly
		if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(World, ARoomBase::StaticClass(), FoundActors);
			
			// Map room name to type
			ERoomType TargetType = ERoomType::EngineeringBay;
			if (RoomName.Equals("MedicalBay", ESearchCase::IgnoreCase))
				TargetType = ERoomType::MedicalBay;
			else if (RoomName.Equals("EngineeringBay", ESearchCase::IgnoreCase))
				TargetType = ERoomType::EngineeringBay;
			else if (RoomName.Equals("CombatArena", ESearchCase::IgnoreCase))
				TargetType = ERoomType::CombatArena;
			else if (RoomName.Equals("CargoHold", ESearchCase::IgnoreCase))
				TargetType = ERoomType::CargoHold;
			else if (RoomName.Equals("Bridge", ESearchCase::IgnoreCase))
				TargetType = ERoomType::Bridge;
			
			// Find and teleport to room
			for (AActor* Actor : FoundActors)
			{
				if (ARoomBase* Room = Cast<ARoomBase>(Actor))
				{
					if (Room->RoomTypeForTesting == TargetType)
					{
						Room->TeleportPlayerToRoom();
						
						// Spawn enemy after teleport
						FTimerHandle SpawnTimer;
						World->GetTimerManager().SetTimer(SpawnTimer, [Room, World]()
						{
							Room->SpawnRoomEnemy();
							PHASE3_LOG("Enemy spawned in room");
						}, 1.0f, false);
						
						PHASE3_LOG(FString::Printf(TEXT("Teleported to %s!"), *RoomName));
						return;
					}
				}
			}
			
			PHASE3_ERROR(FString::Printf(TEXT("Room '%s' not found in level"), *RoomName));
		}
	}
}

void UPhase3ConsoleCommands::ResetRooms(const TArray<FString>& Args)
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->ResetAllRooms();
		PHASE3_LOG("All rooms reset");
	}
	else
	{
		// Reset rooms directly
		if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(World, ARoomBase::StaticClass(), FoundActors);
			
			int32 RoomCount = 0;
			for (AActor* Actor : FoundActors)
			{
				if (ARoomBase* Room = Cast<ARoomBase>(Actor))
				{
					Room->ResetRoom();
					RoomCount++;
				}
			}
			
			PHASE3_LOG(FString::Printf(TEXT("Reset %d rooms"), RoomCount));
		}
	}
}

void UPhase3ConsoleCommands::DebugRooms(const TArray<FString>& Args)
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->DebugRooms();
	}
	else
	{
		// Debug rooms directly
		if (UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GetWorldContexts()[0].World(), EGetWorldErrorMode::ReturnNull))
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(World, ARoomBase::StaticClass(), FoundActors);
			
			PHASE3_LOG(FString::Printf(TEXT("=== ROOM DEBUG INFO ===")));
			PHASE3_LOG(FString::Printf(TEXT("Found %d rooms in level"), FoundActors.Num()));
			
			for (AActor* Actor : FoundActors)
			{
				if (ARoomBase* Room = Cast<ARoomBase>(Actor))
				{
					const UEnum* EnumPtr = StaticEnum<ERoomType>();
					FString RoomTypeName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)Room->RoomTypeForTesting) : TEXT("Unknown");
					
					PHASE3_LOG(FString::Printf(TEXT("Room: %s"), *RoomTypeName));
					PHASE3_LOG(FString::Printf(TEXT("  - Position: %s"), *Room->GetActorLocation().ToString()));
					PHASE3_LOG(FString::Printf(TEXT("  - Active: %s"), Room->IsRoomActive() ? TEXT("YES") : TEXT("NO")));
					PHASE3_LOG(FString::Printf(TEXT("  - Completed: %s"), Room->bTestRoomCompleted ? TEXT("YES") : TEXT("NO")));
				}
			}
		}
	}
}

// ========================================
// RUN SYSTEM IMPLEMENTATION
// ========================================

void UPhase3ConsoleCommands::StartTestRun(const TArray<FString>& Args)
{
	// Reset run tracking
	CurrentRunIndex = 0;
	CurrentSpawnedEnemy = nullptr;
	
	// Clear any existing enemy check timer
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
			{
				if (EnemyCheckTimer.IsValid())
				{
					Context.World()->GetTimerManager().ClearTimer(EnemyCheckTimer);
				}
				break;
			}
		}
	}
	
	// Create randomized room order
	CurrentRunOrder.Empty();
	CurrentRunOrder.Add(ERoomType::EngineeringBay);
	CurrentRunOrder.Add(ERoomType::CombatArena);
	CurrentRunOrder.Add(ERoomType::MedicalBay);
	CurrentRunOrder.Add(ERoomType::CargoHold);
	CurrentRunOrder.Add(ERoomType::Bridge);
	
	// Shuffle the array (Fisher-Yates shuffle)
	for (int32 i = CurrentRunOrder.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		CurrentRunOrder.Swap(i, j);
	}
	
	// Display run order
	PHASE3_LOG("=== STARTING NEW RUN ===");
	PHASE3_LOG("Room Order:");
	for (int32 i = 0; i < CurrentRunOrder.Num(); i++)
	{
		const UEnum* EnumPtr = StaticEnum<ERoomType>();
		FString RoomName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentRunOrder[i]) : TEXT("Unknown");
		PHASE3_LOG(FString::Printf(TEXT("  Level %d: %s"), i + 1, *RoomName));
	}
	
	// Start in first room
	PHASE3_LOG("Teleporting to first room...");
	NextRoom(TArray<FString>());
}

void UPhase3ConsoleCommands::NextRoom(const TArray<FString>& Args)
{
	if (CurrentRunOrder.Num() == 0)
	{
		PHASE3_ERROR("No run in progress! Use Atlas.Phase3.StartTestRun first");
		return;
	}
	
	if (CurrentRunIndex >= CurrentRunOrder.Num())
	{
		PHASE3_LOG("=== RUN COMPLETE! ===");
		PHASE3_LOG("Congratulations! You completed all 5 rooms!");
		CurrentRunOrder.Empty();
		CurrentRunIndex = 0;
		return;
	}
	
	// Get current room type
	ERoomType CurrentRoomType = CurrentRunOrder[CurrentRunIndex];
	const UEnum* EnumPtr = StaticEnum<ERoomType>();
	FString RoomName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentRoomType) : TEXT("Unknown");
	
	PHASE3_LOG(FString::Printf(TEXT("=== LEVEL %d of 5 ==="), CurrentRunIndex + 1));
	PHASE3_LOG(FString::Printf(TEXT("Entering: %s"), *RoomName));
	
	// Find and teleport to room - use improved world detection
	UWorld* World = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
			{
				World = Context.World();
				break;
			}
		}
	}
	
	if (World)
	{
		// Check for player controller first
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			PHASE3_ERROR("No player controller found! Make sure you're in PIE (Play In Editor) mode.");
			PHASE3_ERROR("Press Play in the editor first, then use console commands.");
			CurrentRunOrder.Empty();
			CurrentRunIndex = 0;
			return;
		}
		
		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn)
		{
			PHASE3_ERROR("No player pawn found! Make sure your GameMode spawns a player character.");
			CurrentRunOrder.Empty();
			CurrentRunIndex = 0;
			return;
		}
		
		// Clean up previous enemy if exists
		if (CurrentSpawnedEnemy && IsValid(CurrentSpawnedEnemy))
		{
			CurrentSpawnedEnemy->Destroy();
			CurrentSpawnedEnemy = nullptr;
		}
		
		// Find room and teleport
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, ARoomBase::StaticClass(), FoundActors);
		
		for (AActor* Actor : FoundActors)
		{
			if (ARoomBase* Room = Cast<ARoomBase>(Actor))
			{
				if (Room->RoomTypeForTesting == CurrentRoomType)
				{
					// Teleport player
					Room->TeleportPlayerToRoom();
					
					// Spawn enemy after delay
					FTimerHandle SpawnTimer;
					FTimerDelegate SpawnDelegate;
					SpawnDelegate.BindLambda([Room, World, RoomName]()
					{
						// Spawn a default enemy (you can customize per room)
						if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
						{
							FVector SpawnLocation = Room->GetActorLocation() + FVector(0, 500, 200);
							FRotator SpawnRotation = FRotator::ZeroRotator;
							
							// Try to spawn an enemy character
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
							
							// Spawn BP_Enemy blueprint
							UClass* EnemyBP = LoadClass<APawn>(nullptr, TEXT("/Game/blueprints/BP_Enemy.BP_Enemy_C"));
							if (EnemyBP)
							{
								PHASE3_LOG(FString::Printf(TEXT("Loaded EnemyBP: %s"), *EnemyBP->GetName()));
								
								if (AActor* Enemy = World->SpawnActor<AActor>(EnemyBP, SpawnLocation, SpawnRotation, SpawnParams))
								{
									UPhase3ConsoleCommands::CurrentSpawnedEnemy = Enemy;
									PHASE3_LOG(FString::Printf(TEXT("Enemy spawned in %s! Class: %s"), *RoomName, *Enemy->GetClass()->GetName()));
									
									// Validate enemy has health component
									if (UHealthComponent* HealthComp = Enemy->FindComponentByClass<UHealthComponent>())
									{
										PHASE3_LOG(FString::Printf(TEXT("Enemy Health: %.1f/%.1f"), HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth()));
									}
									else if (AGameCharacterBase* EnemyChar = Cast<AGameCharacterBase>(Enemy))
									{
										if (UHealthComponent* CharHealthComp = EnemyChar->GetHealthComponent())
										{
											PHASE3_LOG(FString::Printf(TEXT("Enemy Health (GameChar): %.1f/%.1f"), CharHealthComp->GetCurrentHealth(), CharHealthComp->GetMaxHealth()));
										}
										else
										{
											PHASE3_ERROR("Enemy spawned but has no HealthComponent!");
										}
									}
									else
									{
										PHASE3_ERROR("Enemy spawned but cannot find HealthComponent!");
									}
									
									PHASE3_LOG("Defeat the enemy to progress!");
									
									// Start checking for enemy death every second
									FTimerDelegate EnemyCheckDelegate;
									EnemyCheckDelegate.BindLambda([World]()
									{
										UPhase3ConsoleCommands::CheckEnemyStatus(World);
									});
									World->GetTimerManager().SetTimer(UPhase3ConsoleCommands::EnemyCheckTimer, EnemyCheckDelegate, 1.0f, true);
									PHASE3_LOG("Enemy death monitoring started (checking every 1.0s)");
								}
								else
								{
									PHASE3_ERROR("Failed to spawn enemy actor! Check spawn location and collision.");
								}
							}
							else
							{
								PHASE3_ERROR("Failed to load BP_Enemy blueprint! Check path: /Game/blueprints/BP_Enemy");
								PHASE3_ERROR("Alternative paths to try:");
								PHASE3_ERROR("  /Game/Blueprints/BP_Enemy.BP_Enemy_C");
								PHASE3_ERROR("  /Game/Characters/BP_Enemy.BP_Enemy_C");
								PHASE3_ERROR("  /Game/Enemy/BP_Enemy.BP_Enemy_C");
							}
						}
					});
					World->GetTimerManager().SetTimer(SpawnTimer, SpawnDelegate, 1.5f, false);
					
					return;
				}
			}
		}
		
		PHASE3_ERROR("Could not find room for current level!");
	}
}

void UPhase3ConsoleCommands::CheckSetup(const TArray<FString>& Args)
{
	PHASE3_LOG("=== CHECKING GAME SETUP ===");
	
	// Check world - try multiple methods
	UWorld* World = nullptr;
	
	// Method 1: Get current PIE world
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				World = Context.World();
				PHASE3_LOG("✓ Found PIE World");
				break;
			}
		}
		
		// Method 2: Try game world if PIE not found
		if (!World)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::Game && Context.World())
				{
					World = Context.World();
					PHASE3_LOG("✓ Found Game World");
					break;
				}
			}
		}
		
		// Method 3: Fallback to any world
		if (!World && GEngine->GetWorldContexts().Num() > 0)
		{
			World = GEngine->GetWorldContexts()[0].World();
			PHASE3_LOG("✓ Found World (fallback)");
		}
	}
	
	if (!World)
	{
		PHASE3_ERROR("✗ No World found! Are you in the editor?");
		return;
	}
	
	PHASE3_LOG(FString::Printf(TEXT("✓ World found: %s"), *World->GetName()));
	PHASE3_LOG(FString::Printf(TEXT("  World Type: %d"), (int32)World->WorldType));
	
	// Check player controller
	APlayerController* PC = World->GetFirstPlayerController();
	
	// Debug info about controllers
	TArray<APlayerController*> AllControllers;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* Controller = Iterator->Get())
		{
			AllControllers.Add(Controller);
		}
	}
	
	PHASE3_LOG(FString::Printf(TEXT("Found %d player controllers total"), AllControllers.Num()));
	
	if (!PC)
	{
		PHASE3_ERROR("✗ No Player Controller found!");
		PHASE3_ERROR("  → You need to press PLAY in the editor first");
		PHASE3_ERROR("  → Then open console with ~ key");
		
		if (AllControllers.Num() > 0)
		{
			PHASE3_LOG("Available controllers:");
			for (int32 i = 0; i < AllControllers.Num(); i++)
			{
				PHASE3_LOG(FString::Printf(TEXT("  %d: %s"), i, *AllControllers[i]->GetName()));
			}
			PC = AllControllers[0]; // Use first available
			PHASE3_LOG("Using first available controller for testing");
		}
	}
	else
	{
		PHASE3_LOG("✓ Player Controller found");
		
		// Check player pawn
		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn)
		{
			PHASE3_ERROR("✗ No Player Pawn found!");
			PHASE3_ERROR("  → Make sure your GameMode spawns a player character");
			PHASE3_ERROR("  → Check World Settings -> GameMode Override");
		}
		else
		{
			PHASE3_LOG(FString::Printf(TEXT("✓ Player Pawn found: %s"), *PlayerPawn->GetName()));
			
			// Check if it's a character
			if (AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(PlayerPawn))
			{
				PHASE3_LOG("✓ Player is GameCharacterBase");
			}
			else if (ACharacter* Char = Cast<ACharacter>(PlayerPawn))
			{
				PHASE3_LOG("✓ Player is Character (not GameCharacterBase)");
			}
			else
			{
				PHASE3_LOG("⚠ Player is Pawn but not Character");
			}
		}
	}
	
	// Check rooms
	TArray<AActor*> FoundRooms;
	UGameplayStatics::GetAllActorsOfClass(World, ARoomBase::StaticClass(), FoundRooms);
	
	if (FoundRooms.Num() == 0)
	{
		PHASE3_ERROR("✗ No rooms found in level!");
		PHASE3_ERROR("  → Add room blueprints to your level");
		PHASE3_ERROR("  → Make sure they inherit from RoomBase");
	}
	else
	{
		PHASE3_LOG(FString::Printf(TEXT("✓ Found %d rooms:"), FoundRooms.Num()));
		
		// List all rooms
		for (AActor* Actor : FoundRooms)
		{
			if (ARoomBase* Room = Cast<ARoomBase>(Actor))
			{
				const UEnum* EnumPtr = StaticEnum<ERoomType>();
				FString RoomName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)Room->RoomTypeForTesting) : TEXT("Unknown");
				PHASE3_LOG(FString::Printf(TEXT("  - %s at %s"), *RoomName, *Room->GetActorLocation().ToString()));
			}
		}
	}
	
	// Summary
	PHASE3_LOG("=== SETUP SUMMARY ===");
	bool bCanRun = World && PC && PC->GetPawn() && (FoundRooms.Num() > 0);
	if (bCanRun)
	{
		PHASE3_LOG("✓ Setup is READY! You can use Atlas.Phase3.StartTestRun");
	}
	else
	{
		PHASE3_ERROR("✗ Setup is NOT ready. Fix the issues above.");
		PHASE3_ERROR("Common fixes:");
		PHASE3_ERROR("  1. Press PLAY button in editor");
		PHASE3_ERROR("  2. Open console with ~ key");
		PHASE3_ERROR("  3. Type: Atlas.Phase3.StartTestRun");
	}
}

void UPhase3ConsoleCommands::CompleteAndProgress(const TArray<FString>& Args)
{
	if (CurrentRunOrder.Num() == 0)
	{
		PHASE3_ERROR("No run in progress!");
		return;
	}
	
	// Clean up current enemy and timer
	if (CurrentSpawnedEnemy && IsValid(CurrentSpawnedEnemy))
	{
		CurrentSpawnedEnemy->Destroy();
		CurrentSpawnedEnemy = nullptr;
	}
	
	// Clear enemy check timer
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
			{
				if (EnemyCheckTimer.IsValid())
				{
					Context.World()->GetTimerManager().ClearTimer(EnemyCheckTimer);
				}
				break;
			}
		}
	}
	
	// Mark current room complete
	const UEnum* EnumPtr = StaticEnum<ERoomType>();
	FString CompletedRoom = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentRunOrder[CurrentRunIndex]) : TEXT("Unknown");
	PHASE3_LOG(FString::Printf(TEXT("Room Complete: %s"), *CompletedRoom));
	
	// Progress to next room
	CurrentRunIndex++;
	
	if (CurrentRunIndex >= CurrentRunOrder.Num())
	{
		// Run complete!
		PHASE3_LOG("=== RUN COMPLETE! ===");
		PHASE3_LOG("Congratulations! You defeated all 5 enemies!");
		PHASE3_LOG("Final Stats:");
		PHASE3_LOG(FString::Printf(TEXT("  Rooms Cleared: %d"), CurrentRunOrder.Num()));
		PHASE3_LOG("  Status: VICTORIOUS!");
		
		// Reset for next run
		CurrentRunOrder.Empty();
		CurrentRunIndex = 0;
		CurrentSpawnedEnemy = nullptr;
	}
	else
	{
		// Show progress
		PHASE3_LOG(FString::Printf(TEXT("Progress: %d/%d rooms complete"), CurrentRunIndex, CurrentRunOrder.Num()));
		PHASE3_LOG("Transitioning to next room in 2 seconds...");
		
		// Auto-progress to next room after delay - use improved world detection
		UWorld* World = nullptr;
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
				{
					World = Context.World();
					break;
				}
			}
		}
		
		if (World)
		{
			FTimerHandle TransitionTimer;
			FTimerDelegate TransitionDelegate;
			TransitionDelegate.BindLambda([]()
			{
				UPhase3ConsoleCommands::NextRoom(TArray<FString>());
			});
			World->GetTimerManager().SetTimer(TransitionTimer, TransitionDelegate, 2.0f, false);
		}
	}
}

void UPhase3ConsoleCommands::CheckEnemyStatus(UWorld* World)
{
	// Debug: Show we're checking
	static int32 CheckCount = 0;
	CheckCount++;
	
	// Check if enemy exists and is valid first
	if (!CurrentSpawnedEnemy || !IsValid(CurrentSpawnedEnemy))
	{
		// Enemy was destroyed - auto progress!
		PHASE3_LOG("Enemy destroyed! Auto-progressing to next room...");
		
		// Clear the timer
		if (World && EnemyCheckTimer.IsValid())
		{
			World->GetTimerManager().ClearTimer(EnemyCheckTimer);
		}
		
		// Auto-progress to next room
		CompleteAndProgress(TArray<FString>());
		return;
	}
	
	// Check if enemy is dead (health <= 0) using multiple methods
	bool bEnemyIsDead = false;
	FString DeathCheckMethod = TEXT("None");
	float CurrentHealth = -1.0f;
	float MaxHealth = -1.0f;
	
	// Method 1: Try casting to GameCharacterBase and check IHealthInterface
	if (AGameCharacterBase* EnemyCharacter = Cast<AGameCharacterBase>(CurrentSpawnedEnemy))
	{
		if (UHealthComponent* HealthComp = EnemyCharacter->GetHealthComponent())
		{
			bEnemyIsDead = HealthComp->IsDead();
			CurrentHealth = HealthComp->GetCurrentHealth();
			MaxHealth = HealthComp->GetMaxHealth();
			DeathCheckMethod = TEXT("GameCharacterBase->HealthComponent");
		}
		else
		{
			// Fallback: use interface method
			bEnemyIsDead = EnemyCharacter->Execute_IsDead(EnemyCharacter);
			DeathCheckMethod = TEXT("GameCharacterBase->IHealthInterface");
		}
	}
	// Method 2: Try direct HealthComponent lookup
	else if (UHealthComponent* HealthComp = CurrentSpawnedEnemy->FindComponentByClass<UHealthComponent>())
	{
		bEnemyIsDead = HealthComp->IsDead();
		CurrentHealth = HealthComp->GetCurrentHealth();
		MaxHealth = HealthComp->GetMaxHealth();
		DeathCheckMethod = TEXT("FindComponentByClass<UHealthComponent>");
	}
	// Method 3: Try IHealthInterface if implemented
	else if (CurrentSpawnedEnemy->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
	{
		bEnemyIsDead = IHealthInterface::Execute_IsDead(CurrentSpawnedEnemy);
		DeathCheckMethod = TEXT("IHealthInterface");
	}
	else
	{
		DeathCheckMethod = TEXT("No valid method found!");
	}
	
	// Log status every 5 checks (every 5 seconds) or if enemy is dead
	if (CheckCount % 5 == 0 || bEnemyIsDead)
	{
		PHASE3_LOG(FString::Printf(TEXT("Enemy Status Check #%d: Method=%s, Health=%.1f/%.1f, IsDead=%s"), 
			CheckCount, *DeathCheckMethod, CurrentHealth, MaxHealth, bEnemyIsDead ? TEXT("TRUE") : TEXT("FALSE")));
	}
	
	if (bEnemyIsDead)
	{
		// Enemy is dead - auto progress!
		PHASE3_LOG("Enemy defeated! Auto-progressing to next room...");
		
		// Clear the timer
		if (World && EnemyCheckTimer.IsValid())
		{
			World->GetTimerManager().ClearTimer(EnemyCheckTimer);
		}
		
		// Reset check count for next enemy
		CheckCount = 0;
		
		// Auto-progress to next room
		CompleteAndProgress(TArray<FString>());
	}
}