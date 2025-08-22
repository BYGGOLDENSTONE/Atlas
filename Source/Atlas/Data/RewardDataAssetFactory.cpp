#include "RewardDataAssetFactory.h"
#include "Atlas/Data/ActionDataAsset.h"

// ========================================
// FACTORY IMPLEMENTATION
// ========================================

void URewardDataAssetFactory::CreateAllRewardDefinitions()
{
	// Defense Category
	CreateImprovedBlockReward();
	CreateParryMasterReward();
	CreateCounterStrikeReward();
	CreateIronSkinReward();
	CreateLastStandReward();
	
	// Offense Category
	CreateSharpBladeReward();
	CreateHeavyImpactReward();
	CreateBleedingStrikesReward();
	CreateExecutionerReward();
	CreateRapidStrikesReward();
	
	// Passive Stats
	CreateVitalityReward();
	CreateSwiftnessReward();
	CreateHeavyweightReward();
	CreateRegenerationReward();
	CreateFortitudeReward();
	
	// Passive Abilities
	CreateSecondWindReward();
	CreateVampirismReward();
	CreateBerserkerReward();
	CreateMomentumReward();
	CreateStationShieldReward();
	
	// Interactables
	CreateExplosiveValvesReward();
	CreateGravityWellsReward();
	CreateTurretHackReward();
	CreateEmergencyVentReward();
	CreatePowerSurgeReward();
}

URewardDataAsset* URewardDataAssetFactory::CreateRewardBase(const FString& Name, ERewardCategory Category)
{
	URewardDataAsset* NewReward = NewObject<URewardDataAsset>(GetTransientPackage(), FName(*Name));
	NewReward->Category = Category;
	NewReward->bUnlockedByDefault = true; // For Phase 3, all rewards start unlocked
	return NewReward;
}

// ========================================
// DEFENSE CATEGORY
// ========================================

URewardDataAsset* URewardDataAssetFactory::CreateImprovedBlockReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Defense_ImprovedBlock", ERewardCategory::Defense);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.ImprovedBlock");
	Reward->RewardName = FText::FromString("Improved Block");
	Reward->Description = FText::FromString("Enhances your blocking ability, reducing more damage with each stack");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {0.6f, 0.7f, 0.8f}; // 60%, 70%, 80% damage reduction
	Reward->bReplacesExistingAbility = true;
	Reward->ReplacesActionTag = FGameplayTag::RequestGameplayTag("Action.Combat.Block");
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateParryMasterReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Defense_ParryMaster", ERewardCategory::Defense);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.ParryMaster");
	Reward->RewardName = FText::FromString("Parry Master");
	Reward->Description = FText::FromString("Extends parry timing windows, making perfect parries easier");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.5f, 2.0f}; // 150%, 200% window size
	
	Reward->PassiveParameters.Add("ParryWindowExtension", 0.1f);
	Reward->PassiveParameters.Add("LateParryWindowExtension", 0.05f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateCounterStrikeReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Defense_CounterStrike", ERewardCategory::Defense);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.CounterStrike");
	Reward->RewardName = FText::FromString("Counter Strike");
	Reward->Description = FText::FromString("Automatically counter-attack after a successful parry");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Counter.Auto");
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateIronSkinReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Defense_IronSkin", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.IronSkin");
	Reward->RewardName = FText::FromString("Iron Skin");
	Reward->Description = FText::FromString("Reduces all incoming damage by a percentage");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	Reward->StatModifiers.Add("DamageReduction", 0.1f); // 10% per stack
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateLastStandReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Defense_LastStand", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.LastStand");
	Reward->RewardName = FText::FromString("Last Stand");
	Reward->Description = FText::FromString("Blocks cannot be broken when health is below 25%");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.LastStand");
	
	Reward->PassiveParameters.Add("HealthThreshold", 0.25f);
	Reward->PassiveParameters.Add("BlockBreakImmunity", 1.0f);
	
	return Reward;
}

// ========================================
// OFFENSE CATEGORY
// ========================================

URewardDataAsset* URewardDataAssetFactory::CreateSharpBladeReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Offense_SharpBlade", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Offense.SharpBlade");
	Reward->RewardName = FText::FromString("Sharp Blade");
	Reward->Description = FText::FromString("Increases all melee damage dealt");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.75f, 2.5f};
	
	Reward->StatModifiers.Add("DamageMultiplier", 0.2f); // 20% per base stack
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateHeavyImpactReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Offense_HeavyImpact", ERewardCategory::Offense);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Offense.HeavyImpact");
	Reward->RewardName = FText::FromString("Heavy Impact");
	Reward->Description = FText::FromString("Heavy attacks create AOE shockwaves on impact");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 2;
	Reward->bReplacesExistingAbility = true;
	Reward->ReplacesActionTag = FGameplayTag::RequestGameplayTag("Action.Combat.HeavyAttack");
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateBleedingStrikesReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Offense_BleedingStrikes", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Offense.BleedingStrikes");
	Reward->RewardName = FText::FromString("Bleeding Strikes");
	Reward->Description = FText::FromString("All attacks apply bleeding damage over time");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.DOT.Bleed");
	
	Reward->PassiveParameters.Add("BleedDamage", 2.0f);
	Reward->PassiveParameters.Add("BleedDuration", 5.0f);
	Reward->PassiveParameters.Add("BleedTickRate", 0.5f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateExecutionerReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Offense_Executioner", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Offense.Executioner");
	Reward->RewardName = FText::FromString("Executioner");
	Reward->Description = FText::FromString("Deal double damage to stunned or staggered enemies");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Execution");
	
	Reward->PassiveParameters.Add("StunnedDamageMultiplier", 2.0f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateRapidStrikesReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Offense_RapidStrikes", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Offense.RapidStrikes");
	Reward->RewardName = FText::FromString("Rapid Strikes");
	Reward->Description = FText::FromString("Increases attack speed for all melee attacks");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	Reward->StatModifiers.Add("AttackSpeed", 0.3f); // 30% faster base
	
	return Reward;
}

// ========================================
// PASSIVE STATS CATEGORY
// ========================================

URewardDataAsset* URewardDataAssetFactory::CreateVitalityReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Passive_Vitality", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Passive.Vitality");
	Reward->RewardName = FText::FromString("Vitality");
	Reward->Description = FText::FromString("Increases maximum health");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	Reward->StatModifiers.Add("MaxHealth", 50.0f); // +50 HP base
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateSwiftnessReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Passive_Swiftness", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Passive.Swiftness");
	Reward->RewardName = FText::FromString("Swiftness");
	Reward->Description = FText::FromString("Increases movement speed");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.0f, 1.5f};
	
	Reward->StatModifiers.Add("MoveSpeed", 0.2f); // 20% faster
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateHeavyweightReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Passive_Heavyweight", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Passive.Heavyweight");
	Reward->RewardName = FText::FromString("Heavyweight");
	Reward->Description = FText::FromString("Increases knockback dealt and reduces knockback received");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.0f, 1.5f};
	
	Reward->StatModifiers.Add("KnockbackForce", 0.5f); // 50% more knockback
	Reward->StatModifiers.Add("KnockbackResistance", 0.25f); // 25% less knockback taken
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateRegenerationReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Passive_Regeneration", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Passive.Regeneration");
	Reward->RewardName = FText::FromString("Regeneration");
	Reward->Description = FText::FromString("Slowly regenerate health when out of combat");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.0f, 1.5f};
	
	Reward->StatModifiers.Add("HealthRegenRate", 2.0f); // 2 HP/sec
	Reward->StatModifiers.Add("CombatRegenDelay", 3.0f); // 3 second delay after combat
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateFortitudeReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Passive_Fortitude", ERewardCategory::PassiveStats);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Passive.Fortitude");
	Reward->RewardName = FText::FromString("Fortitude");
	Reward->Description = FText::FromString("Increases maximum poise and poise regeneration rate");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	Reward->StatModifiers.Add("PoiseMax", 50.0f); // +50 poise
	Reward->StatModifiers.Add("PoiseRegenRate", 0.5f); // 50% faster regen
	
	return Reward;
}

// ========================================
// PASSIVE ABILITIES CATEGORY
// ========================================

URewardDataAsset* URewardDataAssetFactory::CreateSecondWindReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Ability_SecondWind", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Ability.SecondWind");
	Reward->RewardName = FText::FromString("Second Wind");
	Reward->Description = FText::FromString("Upon death, revive with 50% health once per run (consumes reward)");
	Reward->SlotCost = 3;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Revive.Once");
	
	Reward->PassiveParameters.Add("ReviveHealth", 0.5f);
	Reward->PassiveParameters.Add("InvulnerabilityDuration", 2.0f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateVampirismReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Ability_Vampirism", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Ability.Vampirism");
	Reward->RewardName = FText::FromString("Vampirism");
	Reward->Description = FText::FromString("Heal for a percentage of damage dealt");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.0f, 1.5f};
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Lifesteal");
	
	Reward->PassiveParameters.Add("LifestealPercent", 0.1f); // 10% lifesteal
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateBerserkerReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Ability_Berserker", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Ability.Berserker");
	Reward->RewardName = FText::FromString("Berserker");
	Reward->Description = FText::FromString("Deal 50% more damage when below 30% health");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Berserker");
	
	Reward->PassiveParameters.Add("HealthThreshold", 0.3f);
	Reward->PassiveParameters.Add("DamageBonus", 0.5f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateMomentumReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Ability_Momentum", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Ability.Momentum");
	Reward->RewardName = FText::FromString("Momentum");
	Reward->Description = FText::FromString("Each successful hit increases damage, resets on miss");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Momentum");
	
	Reward->PassiveParameters.Add("DamageIncreasePerHit", 0.05f); // 5% per hit
	Reward->PassiveParameters.Add("MaxStacks", 10.0f);
	Reward->PassiveParameters.Add("StackDuration", 5.0f);
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateStationShieldReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Ability_StationShield", ERewardCategory::PassiveAbility);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Ability.StationShield");
	Reward->RewardName = FText::FromString("Station Shield");
	Reward->Description = FText::FromString("50% of damage taken is redirected to station integrity");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->PassiveAbilityTag = FGameplayTag::RequestGameplayTag("Ability.StationShield");
	
	Reward->PassiveParameters.Add("DamageToStation", 0.5f);
	
	return Reward;
}

// ========================================
// INTERACTABLES CATEGORY
// ========================================

URewardDataAsset* URewardDataAssetFactory::CreateExplosiveValvesReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Interact_ExplosiveValves", ERewardCategory::Interactable);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Interact.ExplosiveValves");
	Reward->RewardName = FText::FromString("Explosive Valves");
	Reward->Description = FText::FromString("Interact with valves to cause explosive damage in an area");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 2;
	Reward->StackMultipliers = {1.0f, 1.5f};
	Reward->InteractableType = FGameplayTag::RequestGameplayTag("Interact.Valve.Explosive");
	Reward->InteractionRange = 800.0f;
	Reward->InteractableCooldown = 10.0f;
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateGravityWellsReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Interact_GravityWells", ERewardCategory::Interactable);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Interact.GravityWells");
	Reward->RewardName = FText::FromString("Gravity Wells");
	Reward->Description = FText::FromString("Create gravity wells that pull and slow enemies");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->InteractableType = FGameplayTag::RequestGameplayTag("Interact.Gravity.Well");
	Reward->InteractionRange = 1000.0f;
	Reward->InteractableCooldown = 15.0f;
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateTurretHackReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Interact_TurretHack", ERewardCategory::Interactable);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Interact.TurretHack");
	Reward->RewardName = FText::FromString("Turret Hack");
	Reward->Description = FText::FromString("Hack station turrets to fight for you");
	Reward->SlotCost = 2;
	Reward->MaxStackLevel = 1;
	Reward->InteractableType = FGameplayTag::RequestGameplayTag("Interact.Turret.Hack");
	Reward->InteractionRange = 500.0f;
	Reward->InteractableCooldown = 20.0f;
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreateEmergencyVentReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Interact_EmergencyVent", ERewardCategory::Interactable);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Interact.EmergencyVent");
	Reward->RewardName = FText::FromString("Emergency Vent");
	Reward->Description = FText::FromString("Trigger emergency vents to launch enemies");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 2;
	Reward->InteractableType = FGameplayTag::RequestGameplayTag("Interact.Vent.Emergency");
	Reward->InteractionRange = 600.0f;
	Reward->InteractableCooldown = 8.0f;
	
	return Reward;
}

URewardDataAsset* URewardDataAssetFactory::CreatePowerSurgeReward()
{
	URewardDataAsset* Reward = CreateRewardBase("DA_Reward_Interact_PowerSurge", ERewardCategory::Interactable);
	
	Reward->RewardTag = FGameplayTag::RequestGameplayTag("Reward.Interact.PowerSurge");
	Reward->RewardName = FText::FromString("Power Surge");
	Reward->Description = FText::FromString("Overload panels to stun and damage nearby enemies");
	Reward->SlotCost = 1;
	Reward->MaxStackLevel = 3;
	Reward->StackMultipliers = {1.0f, 1.5f, 2.0f};
	Reward->InteractableType = FGameplayTag::RequestGameplayTag("Interact.Panel.Overload");
	Reward->InteractionRange = 400.0f;
	Reward->InteractableCooldown = 6.0f;
	
	return Reward;
}

// ========================================
// STORAGE IMPLEMENTATION
// ========================================

void URewardDataAssetStorage::InitializeDefinitions()
{
	// Initialize Defense Rewards
	{
		FRewardDefinition ImprovedBlock;
		ImprovedBlock.AssetName = "DA_Reward_Defense_ImprovedBlock";
		ImprovedBlock.RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.ImprovedBlock");
		ImprovedBlock.RewardName = FText::FromString("Improved Block");
		ImprovedBlock.Description = FText::FromString("Enhances your blocking ability, reducing more damage with each stack");
		ImprovedBlock.Category = ERewardCategory::Defense;
		ImprovedBlock.SlotCost = 1;
		ImprovedBlock.MaxStackLevel = 3;
		ImprovedBlock.StackMultipliers = {0.6f, 0.7f, 0.8f};
		DefenseRewards.Add(ImprovedBlock);
		
		FRewardDefinition ParryMaster;
		ParryMaster.AssetName = "DA_Reward_Defense_ParryMaster";
		ParryMaster.RewardTag = FGameplayTag::RequestGameplayTag("Reward.Defense.ParryMaster");
		ParryMaster.RewardName = FText::FromString("Parry Master");
		ParryMaster.Description = FText::FromString("Extends parry timing windows, making perfect parries easier");
		ParryMaster.Category = ERewardCategory::Defense;
		ParryMaster.SlotCost = 1;
		ParryMaster.MaxStackLevel = 2;
		ParryMaster.StackMultipliers = {1.5f, 2.0f};
		ParryMaster.PassiveParameters.Add("ParryWindowExtension", 0.1f);
		ParryMaster.PassiveParameters.Add("LateParryWindowExtension", 0.05f);
		DefenseRewards.Add(ParryMaster);
		
		// Add other defense rewards...
	}
	
	// Initialize other categories similarly...
}