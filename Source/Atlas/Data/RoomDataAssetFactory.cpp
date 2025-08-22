#include "RoomDataAssetFactory.h"
#include "Atlas/Data/RewardDataAsset.h"

// ========================================
// FACTORY IMPLEMENTATION
// ========================================

void URoomDataAssetFactory::CreateAllRoomDefinitions()
{
	// Create all 5 unique rooms
	CreateEngineeringBayRoom();    // Room A - Defense themed
	CreateMedicalWardRoom();        // Room B - Passive Stats themed
	CreateWeaponsLabRoom();         // Room C - Offense themed
	CreateCommandCenterRoom();      // Room D - Passive Ability themed
	CreateMaintenanceShaftRoom();   // Room E - Interactable themed
}

URoomDataAsset* URoomDataAssetFactory::CreateRoomBase(const FString& Name, ERewardCategory Theme)
{
	URoomDataAsset* NewRoom = NewObject<URoomDataAsset>(GetTransientPackage(), FName(*Name));
	NewRoom->RoomTheme = Theme;
	NewRoom->RewardChoiceCount = 2; // Standard: player chooses between 2 rewards
	return NewRoom;
}

void URoomDataAssetFactory::AddRewardToPool(URoomDataAsset* Room, const FString& RewardAssetPath, 
	float Weight, int32 MinLevel, int32 MaxAppear)
{
	FRewardChoice Choice;
	// In actual implementation, this would load the reward DataAsset
	// Choice.Reward = LoadObject<URewardDataAsset>(nullptr, *RewardAssetPath);
	Choice.SelectionWeight = Weight;
	Choice.MinimumLevel = MinLevel;
	Choice.MaxAppearances = MaxAppear;
	Room->RewardPool.Add(Choice);
}

// ========================================
// ROOM A - ENGINEERING BAY
// ========================================

URoomDataAsset* URoomDataAssetFactory::CreateEngineeringBayRoom()
{
	URoomDataAsset* Room = CreateRoomBase("DA_Room_A_Engineering", ERewardCategory::Defense);
	
	// Room Identity
	Room->RoomID = "Room_A_Engineering";
	Room->RoomName = FText::FromString("Engineering Bay");
	Room->Description = FText::FromString("The heart of the station's mechanical systems. Sparks fly from damaged panels as the chief engineer makes his last stand among the machinery he once maintained.");
	
	// Room Configuration
	Room->Difficulty = ERoomDifficulty::Medium;
	Room->EnvironmentalHazard = ERoomHazard::ElectricalSurges;
	Room->AppearOnLevels = {1, 2, 3};
	Room->RoomSelectionWeight = 1.0f;
	Room->bCanRepeat = false;
	
	// Enemy Configuration
	Room->EnemyName = FText::FromString("Chief Engineer Hayes");
	Room->EnemyDescription = FText::FromString("Once maintained the station, now maintains chaos. Hayes has fortified himself behind layers of energy shields and defensive systems.");
	Room->EnemyBasePower = 3;
	Room->EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Defensive");
	Room->EnemyAbilities = {
		FGameplayTag::RequestGameplayTag("Action.Combat.Block"),
		FGameplayTag::RequestGameplayTag("Action.Shield.Deploy")
	};
	
	// Reward Pool - Defense themed (weight 2.0), with some variety
	// Defense rewards (primary theme)
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_ImprovedBlock", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_ParryMaster", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_CounterStrike", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_IronSkin", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_LastStand", 2.0f, 2); // Min level 2
	
	// Some passive stats for variety
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Vitality", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Fortitude", 1.0f);
	
	// A couple offense options
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_SharpBlade", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_HeavyImpact", 1.0f);
	
	// One interactable
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_ExplosiveValves", 1.0f);
	
	// Environmental Setup
	Room->IntegrityDrainRate = 0.0f; // No passive drain
	Room->TimeLimit = 0.0f; // No time limit
	
	// Dialogue
	Room->IntroDialogue = {
		FText::FromString("You shouldn't have come here. This is MY domain!"),
		FText::FromString("Every system, every circuit - they all answer to ME!")
	};
	Room->VictoryDialogue = {
		FText::FromString("The systems... they're failing... without me...")
	};
	Room->DefeatDialogue = {
		FText::FromString("Another intruder eliminated. The station remains secure.")
	};
	
	return Room;
}

// ========================================
// ROOM B - MEDICAL WARD
// ========================================

URoomDataAsset* URoomDataAssetFactory::CreateMedicalWardRoom()
{
	URoomDataAsset* Room = CreateRoomBase("DA_Room_B_Medical", ERewardCategory::PassiveStats);
	
	// Room Identity
	Room->RoomID = "Room_B_Medical";
	Room->RoomName = FText::FromString("Medical Ward");
	Room->Description = FText::FromString("Where healing became harm. The medical bay reeks of chemicals and decay. Dr. Voss moves between the beds, administering 'treatments' to patients long past saving.");
	
	// Room Configuration
	Room->Difficulty = ERoomDifficulty::Easy;
	Room->EnvironmentalHazard = ERoomHazard::ToxicLeak;
	Room->AppearOnLevels = {1, 2};
	Room->RoomSelectionWeight = 1.2f; // Slightly more common (easier room)
	Room->bCanRepeat = false;
	
	// Enemy Configuration
	Room->EnemyName = FText::FromString("Dr. Voss");
	Room->EnemyDescription = FText::FromString("The cure is worse than the disease. Dr. Voss has turned the medical bay into a laboratory of horrors, using toxic compounds and corrupted healing technology.");
	Room->EnemyBasePower = 2;
	Room->EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Balanced");
	Room->EnemyAbilities = {
		FGameplayTag::RequestGameplayTag("Action.Heal.Self"),
		FGameplayTag::RequestGameplayTag("Action.Poison.Apply")
	};
	
	// Reward Pool - Passive Stats themed (weight 2.0)
	// Passive stats (primary theme)
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Vitality", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Swiftness", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Heavyweight", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Regeneration", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Fortitude", 2.0f);
	
	// Some defense options
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_IronSkin", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_LastStand", 1.0f, 2);
	
	// Passive abilities that fit the theme
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Vampirism", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_SecondWind", 1.0f, 3); // Min level 3
	
	// One offense option
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_BleedingStrikes", 1.0f);
	
	// Environmental Setup
	Room->IntegrityDrainRate = 0.0f;
	Room->TimeLimit = 0.0f;
	
	// Dialogue
	Room->IntroDialogue = {
		FText::FromString("A new patient! Don't worry, I'll take good care of you..."),
		FText::FromString("The treatment may hurt, but pain is just weakness leaving the body!")
	};
	Room->VictoryDialogue = {
		FText::FromString("No... I was so close to the cure...")
	};
	Room->DefeatDialogue = {
		FText::FromString("Another successful treatment. Next patient, please.")
	};
	
	return Room;
}

// ========================================
// ROOM C - WEAPONS LAB
// ========================================

URoomDataAsset* URoomDataAssetFactory::CreateWeaponsLabRoom()
{
	URoomDataAsset* Room = CreateRoomBase("DA_Room_C_Weapons", ERewardCategory::Offense);
	
	// Room Identity
	Room->RoomID = "Room_C_Weapons";
	Room->RoomName = FText::FromString("Weapons Laboratory");
	Room->Description = FText::FromString("Where destruction was perfected. The weapons lab is filled with prototypes and experimental armaments. Commander Rex stands ready, armed with the station's deadliest technology.");
	
	// Room Configuration
	Room->Difficulty = ERoomDifficulty::Hard;
	Room->EnvironmentalHazard = ERoomHazard::None; // Pure combat focus
	Room->AppearOnLevels = {2, 3, 4};
	Room->RoomSelectionWeight = 0.8f; // Less common (harder room)
	Room->bCanRepeat = false;
	
	// Enemy Configuration
	Room->EnemyName = FText::FromString("Commander Rex");
	Room->EnemyDescription = FText::FromString("Armed and extremely dangerous. Rex has access to the station's entire weapons arsenal and isn't afraid to use it.");
	Room->EnemyBasePower = 4;
	Room->EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Aggressive");
	Room->EnemyAbilities = {
		FGameplayTag::RequestGameplayTag("Action.Combat.HeavyAttack"),
		FGameplayTag::RequestGameplayTag("Action.Combo.Triple")
	};
	
	// Reward Pool - Offense themed (weight 2.0)
	// Offense rewards (primary theme)
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_SharpBlade", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_HeavyImpact", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_BleedingStrikes", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_Executioner", 2.0f, 2);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_RapidStrikes", 2.0f);
	
	// Some defense for balance
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_CounterStrike", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_ParryMaster", 1.0f);
	
	// Offensive passive abilities
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Berserker", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Momentum", 1.0f);
	
	// One interactable
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_TurretHack", 1.0f);
	
	// Environmental Setup
	Room->IntegrityDrainRate = 0.0f;
	Room->TimeLimit = 0.0f;
	
	// Dialogue
	Room->IntroDialogue = {
		FText::FromString("Finally, a worthy opponent! Let's see what you're made of!"),
		FText::FromString("I've been waiting for a real fight. Don't disappoint me!")
	};
	Room->VictoryDialogue = {
		FText::FromString("A warrior's death... there is no greater honor...")
	};
	Room->DefeatDialogue = {
		FText::FromString("Too easy. Send me a real challenge next time.")
	};
	
	return Room;
}

// ========================================
// ROOM D - COMMAND CENTER
// ========================================

URoomDataAsset* URoomDataAssetFactory::CreateCommandCenterRoom()
{
	URoomDataAsset* Room = CreateRoomBase("DA_Room_D_Command", ERewardCategory::PassiveAbility);
	
	// Room Identity
	Room->RoomID = "Room_D_Command";
	Room->RoomName = FText::FromString("Command Center");
	Room->Description = FText::FromString("Where order turned to chaos. The command center's screens flicker with corrupted data. Admiral Kronos stands at the helm, determined to go down with his ship.");
	
	// Room Configuration
	Room->Difficulty = ERoomDifficulty::Hard;
	Room->EnvironmentalHazard = ERoomHazard::SystemMalfunction;
	Room->AppearOnLevels = {3, 4, 5};
	Room->RoomSelectionWeight = 0.9f;
	Room->bCanRepeat = false;
	
	// Enemy Configuration
	Room->EnemyName = FText::FromString("Admiral Kronos");
	Room->EnemyDescription = FText::FromString("Captain goes down with the ship. Kronos commands the station's remaining systems, using tactical superiority and orbital strikes.");
	Room->EnemyBasePower = 5;
	Room->EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Tactical");
	Room->EnemyAbilities = {
		FGameplayTag::RequestGameplayTag("Action.Command.Rally"),
		FGameplayTag::RequestGameplayTag("Action.Orbital.Strike")
	};
	
	// Reward Pool - Passive Ability themed (weight 2.0)
	// Passive abilities (primary theme)
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_SecondWind", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Vampirism", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Berserker", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_Momentum", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Ability/DA_Reward_Ability_StationShield", 2.0f);
	
	// Some offense options
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_Executioner", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_RapidStrikes", 1.0f);
	
	// Some defense options
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_IronSkin", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_LastStand", 1.0f);
	
	// One interactable
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_PowerSurge", 1.0f);
	
	// Environmental Setup
	Room->IntegrityDrainRate = 0.0f;
	Room->TimeLimit = 0.0f;
	
	// Dialogue
	Room->IntroDialogue = {
		FText::FromString("You've made it far, but this is where your journey ends."),
		FText::FromString("I am the Admiral of this station. You will show respect... or die!")
	};
	Room->VictoryDialogue = {
		FText::FromString("The station... is yours... Use it... wisely...")
	};
	Room->DefeatDialogue = {
		FText::FromString("The chain of command remains unbroken. You were never a threat.")
	};
	
	return Room;
}

// ========================================
// ROOM E - MAINTENANCE SHAFT
// ========================================

URoomDataAsset* URoomDataAssetFactory::CreateMaintenanceShaftRoom()
{
	URoomDataAsset* Room = CreateRoomBase("DA_Room_E_Maintenance", ERewardCategory::Interactable);
	
	// Room Identity
	Room->RoomID = "Room_E_Maintenance";
	Room->RoomName = FText::FromString("Maintenance Shaft");
	Room->Description = FText::FromString("The station's forgotten underbelly. Narrow corridors wind through exposed machinery. Unit M-471 skitters through the vents, using the environment itself as a weapon.");
	
	// Room Configuration
	Room->Difficulty = ERoomDifficulty::Medium;
	Room->EnvironmentalHazard = ERoomHazard::LowGravity;
	Room->AppearOnLevels = {1, 2, 3, 4, 5}; // Can appear at any level
	Room->RoomSelectionWeight = 1.1f;
	Room->bCanRepeat = false;
	
	// Enemy Configuration
	Room->EnemyName = FText::FromString("Unit M-471");
	Room->EnemyDescription = FText::FromString("Repurposed for destruction. This maintenance drone has gone rogue, using its intimate knowledge of the station's systems against you.");
	Room->EnemyBasePower = 3;
	Room->EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Tactical");
	Room->EnemyAbilities = {
		FGameplayTag::RequestGameplayTag("Action.Combat.Dash"),
		FGameplayTag::RequestGameplayTag("Action.Environment.Use")
	};
	
	// Reward Pool - Interactable themed (weight 2.0)
	// Interactables (primary theme)
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_ExplosiveValves", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_GravityWells", 2.0f, 2);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_TurretHack", 2.0f, 2);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_EmergencyVent", 2.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Interact/DA_Reward_Interact_PowerSurge", 2.0f);
	
	// Some movement-related stats
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Swiftness", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Passive/DA_Reward_Passive_Heavyweight", 1.0f);
	
	// Some defense for survivability
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_ParryMaster", 1.0f);
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Defense/DA_Reward_Defense_CounterStrike", 1.0f);
	
	// One offense option
	AddRewardToPool(Room, "/Game/DataAssets/Rewards/Offense/DA_Reward_Offense_HeavyImpact", 1.0f);
	
	// Environmental Setup
	Room->IntegrityDrainRate = 0.0f;
	Room->TimeLimit = 0.0f;
	
	// Dialogue
	Room->IntroDialogue = {
		FText::FromString("*mechanical whirring* INTRUDER DETECTED. INITIATING ELIMINATION PROTOCOL."),
		FText::FromString("*beeping* MAINTENANCE MODE: AGGRESSIVE. TARGET ACQUIRED.")
	};
	Room->VictoryDialogue = {
		FText::FromString("*sparking* SYSTEMS... FAILING... ERROR... ERROR...")
	};
	Room->DefeatDialogue = {
		FText::FromString("*cheerful beep* MAINTENANCE COMPLETE. AREA SANITIZED.")
	};
	
	return Room;
}

// ========================================
// STORAGE IMPLEMENTATION
// ========================================

void URoomDataAssetStorage::InitializeDefinitions()
{
	AllRooms.Empty();
	
	// Room A - Engineering Bay
	{
		FRoomDefinition RoomA;
		RoomA.AssetName = "DA_Room_A_Engineering";
		RoomA.RoomID = "Room_A_Engineering";
		RoomA.RoomName = FText::FromString("Engineering Bay");
		RoomA.Description = FText::FromString("The heart of the station's mechanical systems");
		RoomA.RoomTheme = ERewardCategory::Defense;
		RoomA.Difficulty = ERoomDifficulty::Medium;
		RoomA.EnvironmentalHazard = ERoomHazard::ElectricalSurges;
		RoomA.AppearOnLevels = {1, 2, 3};
		RoomA.RoomSelectionWeight = 1.0f;
		RoomA.EnemyBlueprintPath = "/Game/Blueprints/Enemies/BP_Enemy_HeavyTechnician";
		RoomA.EnemyName = FText::FromString("Chief Engineer Hayes");
		RoomA.EnemyDescription = FText::FromString("Once maintained the station, now maintains chaos");
		RoomA.EnemyBasePower = 3;
		RoomA.EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Defensive");
		RoomA.RoomLevelPath = "/Game/Maps/Rooms/L_Engineering";
		AllRooms.Add(RoomA);
	}
	
	// Room B - Medical Ward
	{
		FRoomDefinition RoomB;
		RoomB.AssetName = "DA_Room_B_Medical";
		RoomB.RoomID = "Room_B_Medical";
		RoomB.RoomName = FText::FromString("Medical Ward");
		RoomB.Description = FText::FromString("Where healing became harm");
		RoomB.RoomTheme = ERewardCategory::PassiveStats;
		RoomB.Difficulty = ERoomDifficulty::Easy;
		RoomB.EnvironmentalHazard = ERoomHazard::ToxicLeak;
		RoomB.AppearOnLevels = {1, 2};
		RoomB.RoomSelectionWeight = 1.2f;
		RoomB.EnemyBlueprintPath = "/Game/Blueprints/Enemies/BP_Enemy_CorruptedMedic";
		RoomB.EnemyName = FText::FromString("Dr. Voss");
		RoomB.EnemyDescription = FText::FromString("The cure is worse than the disease");
		RoomB.EnemyBasePower = 2;
		RoomB.EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Balanced");
		RoomB.RoomLevelPath = "/Game/Maps/Rooms/L_Medical";
		AllRooms.Add(RoomB);
	}
	
	// Room C - Weapons Lab
	{
		FRoomDefinition RoomC;
		RoomC.AssetName = "DA_Room_C_Weapons";
		RoomC.RoomID = "Room_C_Weapons";
		RoomC.RoomName = FText::FromString("Weapons Laboratory");
		RoomC.Description = FText::FromString("Where destruction was perfected");
		RoomC.RoomTheme = ERewardCategory::Offense;
		RoomC.Difficulty = ERoomDifficulty::Hard;
		RoomC.EnvironmentalHazard = ERoomHazard::None;
		RoomC.AppearOnLevels = {2, 3, 4};
		RoomC.RoomSelectionWeight = 0.8f;
		RoomC.EnemyBlueprintPath = "/Game/Blueprints/Enemies/BP_Enemy_SecurityChief";
		RoomC.EnemyName = FText::FromString("Commander Rex");
		RoomC.EnemyDescription = FText::FromString("Armed and extremely dangerous");
		RoomC.EnemyBasePower = 4;
		RoomC.EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Aggressive");
		RoomC.RoomLevelPath = "/Game/Maps/Rooms/L_Weapons";
		AllRooms.Add(RoomC);
	}
	
	// Room D - Command Center
	{
		FRoomDefinition RoomD;
		RoomD.AssetName = "DA_Room_D_Command";
		RoomD.RoomID = "Room_D_Command";
		RoomD.RoomName = FText::FromString("Command Center");
		RoomD.Description = FText::FromString("Where order turned to chaos");
		RoomD.RoomTheme = ERewardCategory::PassiveAbility;
		RoomD.Difficulty = ERoomDifficulty::Hard;
		RoomD.EnvironmentalHazard = ERoomHazard::SystemMalfunction;
		RoomD.AppearOnLevels = {3, 4, 5};
		RoomD.RoomSelectionWeight = 0.9f;
		RoomD.EnemyBlueprintPath = "/Game/Blueprints/Enemies/BP_Enemy_StationCommander";
		RoomD.EnemyName = FText::FromString("Admiral Kronos");
		RoomD.EnemyDescription = FText::FromString("Captain goes down with the ship");
		RoomD.EnemyBasePower = 5;
		RoomD.EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Tactical");
		RoomD.RoomLevelPath = "/Game/Maps/Rooms/L_Command";
		AllRooms.Add(RoomD);
	}
	
	// Room E - Maintenance Shaft
	{
		FRoomDefinition RoomE;
		RoomE.AssetName = "DA_Room_E_Maintenance";
		RoomE.RoomID = "Room_E_Maintenance";
		RoomE.RoomName = FText::FromString("Maintenance Shaft");
		RoomE.Description = FText::FromString("The station's forgotten underbelly");
		RoomE.RoomTheme = ERewardCategory::Interactable;
		RoomE.Difficulty = ERoomDifficulty::Medium;
		RoomE.EnvironmentalHazard = ERoomHazard::LowGravity;
		RoomE.AppearOnLevels = {1, 2, 3, 4, 5};
		RoomE.RoomSelectionWeight = 1.1f;
		RoomE.EnemyBlueprintPath = "/Game/Blueprints/Enemies/BP_Enemy_MaintenanceDrone";
		RoomE.EnemyName = FText::FromString("Unit M-471");
		RoomE.EnemyDescription = FText::FromString("Repurposed for destruction");
		RoomE.EnemyBasePower = 3;
		RoomE.EnemyAIPreset = FGameplayTag::RequestGameplayTag("AI.Tactical");
		RoomE.RoomLevelPath = "/Game/Maps/Rooms/L_Maintenance";
		AllRooms.Add(RoomE);
	}
}

FRoomDefinition URoomDataAssetStorage::GetRoomByID(FName RoomID) const
{
	for (const FRoomDefinition& Room : AllRooms)
	{
		if (Room.RoomID == RoomID)
		{
			return Room;
		}
	}
	return FRoomDefinition();
}

TArray<FRoomDefinition> URoomDataAssetStorage::GetRoomsForLevel(int32 Level) const
{
	TArray<FRoomDefinition> ValidRooms;
	for (const FRoomDefinition& Room : AllRooms)
	{
		if (Room.AppearOnLevels.Contains(Level))
		{
			ValidRooms.Add(Room);
		}
	}
	return ValidRooms;
}