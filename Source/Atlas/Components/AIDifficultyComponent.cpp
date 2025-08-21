#include "AIDifficultyComponent.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UAIDifficultyComponent::UAIDifficultyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAIDifficultyComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get owner reference
	OwnerCharacter = Cast<AGameCharacterBase>(GetOwner());
	
	// Calculate initial difficulty
	RecalculateDifficulty();
	
	// Apply initial scaling
	ApplyDifficultyScaling();
}

void UAIDifficultyComponent::CalculateDifficulty(int32 PlayerEquippedSlots)
{
	// GDD Rule: Enemy Power = Player Equipped Slots + 1
	PowerLevel = PlayerEquippedSlots + 1;
	
	UE_LOG(LogTemp, Log, TEXT("AI Difficulty calculated - Power Level: %d (Player Slots: %d)"), 
		PowerLevel, PlayerEquippedSlots);
	
	// Apply scaling based on power level
	ApplyPowerLevelScaling(PowerLevel);
	
	// Update behavior based on power level
	if (PowerLevel <= 2)
	{
		// Early game - more defensive
		SetBehaviorPreset(EAIBehaviorPreset::Defensive);
	}
	else if (PowerLevel <= 4)
	{
		// Mid game - balanced
		SetBehaviorPreset(EAIBehaviorPreset::Balanced);
	}
	else
	{
		// Late game - aggressive
		SetBehaviorPreset(EAIBehaviorPreset::Aggressive);
	}
}

void UAIDifficultyComponent::ApplyDifficultyScaling()
{
	if (!OwnerCharacter)
		return;
		
	// Apply health scaling
	if (UHealthComponent* HealthComp = OwnerCharacter->GetHealthComponent())
	{
		float BaseHealth = HealthComp->GetMaxHealth();
		float ScaledHealth = BaseHealth * HealthMultiplier;
		HealthComp->SetMaxHealth(ScaledHealth);
		HealthComp->Heal(ScaledHealth, nullptr); // Set to max health
		
		UE_LOG(LogTemp, Log, TEXT("Applied health scaling: %f -> %f"), BaseHealth, ScaledHealth);
	}
	
	// Apply movement speed scaling
	if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
	{
		float BaseSpeed = 600.0f; // Default walk speed
		MoveComp->MaxWalkSpeed = BaseSpeed * MovementSpeedModifier;
		
		UE_LOG(LogTemp, Log, TEXT("Applied movement speed: %f"), MoveComp->MaxWalkSpeed);
	}
	
	// Note: Damage and other combat modifiers are applied in real-time through getter functions
}

void UAIDifficultyComponent::RecalculateDifficulty()
{
	// Find player and get equipped slots
	int32 PlayerSlots = 0;
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(PC->GetPawn()))
		{
			if (USlotManagerComponent* SlotManager = PlayerChar->FindComponentByClass<USlotManagerComponent>())
			{
				PlayerSlots = SlotManager->GetUsedSlotCount();
			}
		}
	}
	
	CalculateDifficulty(PlayerSlots);
}

void UAIDifficultyComponent::SetBehaviorPreset(EAIBehaviorPreset NewPreset)
{
	CurrentBehaviorPreset = NewPreset;
	UpdateBehaviorFromPreset();
	
	UE_LOG(LogTemp, Log, TEXT("AI Behavior preset changed to: %s"), 
		*UEnum::GetValueAsString(NewPreset));
}

void UAIDifficultyComponent::RecordPlayerAction(FGameplayTag ActionTag)
{
	if (!bEnableAdaptiveAI)
		return;
		
	// Update pattern data
	PlayerPattern.LastPlayerAction = ActionTag;
	
	// Track specific actions
	FString ActionString = ActionTag.ToString();
	if (ActionString.Contains(TEXT("Block")))
	{
		PlayerPattern.BlockCount++;
	}
	else if (ActionString.Contains(TEXT("Parry")))
	{
		PlayerPattern.ParryCount++;
	}
	else if (ActionString.Contains(TEXT("Dash")))
	{
		PlayerPattern.DashCount++;
	}
	else if (ActionString.Contains(TEXT("Attack.Heavy")))
	{
		PlayerPattern.HeavyAttackCount++;
	}
	else if (ActionString.Contains(TEXT("Attack")))
	{
		PlayerPattern.AttackCount++;
	}
	
	// Track reaction time
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastActionTime > 0.0f)
	{
		float ReactionTime = CurrentTime - LastActionTime;
		ReactionTimes.Add(ReactionTime);
		
		// Keep only recent reaction times
		if (ReactionTimes.Num() > 20)
		{
			ReactionTimes.RemoveAt(0);
		}
		
		// Calculate average
		float TotalTime = 0.0f;
		for (float Time : ReactionTimes)
		{
			TotalTime += Time;
		}
		PlayerPattern.AverageReactionTime = TotalTime / ReactionTimes.Num();
	}
	LastActionTime = CurrentTime;
	
	// Check if we should adapt
	int32 TotalActions = PlayerPattern.BlockCount + PlayerPattern.ParryCount + 
	                     PlayerPattern.DashCount + PlayerPattern.AttackCount + 
	                     PlayerPattern.HeavyAttackCount;
	                     
	if (TotalActions >= MinActionsForAdaptation)
	{
		AnalyzePlayerPattern();
	}
}

void UAIDifficultyComponent::AnalyzePlayerPattern()
{
	int32 DefensiveActions = PlayerPattern.BlockCount + PlayerPattern.ParryCount;
	int32 OffensiveActions = PlayerPattern.AttackCount + PlayerPattern.HeavyAttackCount;
	int32 EvasiveActions = PlayerPattern.DashCount;
	
	// Determine most used action type
	if (DefensiveActions > OffensiveActions && DefensiveActions > EvasiveActions)
	{
		PlayerPattern.MostUsedAction = FGameplayTag::RequestGameplayTag(TEXT("Player.Style.Defensive"));
	}
	else if (OffensiveActions > DefensiveActions && OffensiveActions > EvasiveActions)
	{
		PlayerPattern.MostUsedAction = FGameplayTag::RequestGameplayTag(TEXT("Player.Style.Aggressive"));
	}
	else if (EvasiveActions > DefensiveActions && EvasiveActions > OffensiveActions)
	{
		PlayerPattern.MostUsedAction = FGameplayTag::RequestGameplayTag(TEXT("Player.Style.Evasive"));
	}
	else
	{
		PlayerPattern.MostUsedAction = FGameplayTag::RequestGameplayTag(TEXT("Player.Style.Balanced"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("Player pattern analyzed - Style: %s"), 
		*PlayerPattern.MostUsedAction.ToString());
	
	// Adapt strategy
	AdaptToPlayerStrategy();
}

void UAIDifficultyComponent::AdaptToPlayerStrategy()
{
	if (!bEnableAdaptiveAI)
		return;
		
	FString StyleString = PlayerPattern.MostUsedAction.ToString();
	
	if (StyleString.Contains(TEXT("Defensive")))
	{
		CounterDefensivePlayer();
	}
	else if (StyleString.Contains(TEXT("Aggressive")))
	{
		CounterAggressivePlayer();
	}
	else if (StyleString.Contains(TEXT("Evasive")))
	{
		CounterEvasivePlayer();
	}
}

void UAIDifficultyComponent::ResetPatternAnalysis()
{
	PlayerPattern = FPlayerPatternData();
	ReactionTimes.Empty();
	LastActionTime = 0.0f;
	
	UE_LOG(LogTemp, Log, TEXT("Pattern analysis reset"));
}

bool UAIDifficultyComponent::ShouldAttack() const
{
	float AttackWeight = CalculateAttackWeight();
	float RandomValue = FMath::FRandRange(0.0f, 1.0f);
	
	return RandomValue < AttackWeight;
}

bool UAIDifficultyComponent::ShouldDefend() const
{
	float DefenseWeight = CalculateDefenseWeight();
	float RandomValue = FMath::FRandRange(0.0f, 1.0f);
	
	return RandomValue < DefenseWeight;
}

bool UAIDifficultyComponent::ShouldUseAbility() const
{
	float AbilityWeight = CalculateAbilityWeight();
	float RandomValue = FMath::FRandRange(0.0f, 1.0f);
	
	return RandomValue < AbilityWeight;
}

FGameplayTag UAIDifficultyComponent::GetRecommendedAction()
{
	// Check for berserker mode
	if (IsBerserking())
	{
		// Always attack when berserking
		return FMath::FRandRange(0.0f, 1.0f) < 0.3f ? 
			FGameplayTag::RequestGameplayTag(TEXT("Action.Attack.Heavy")) :
			FGameplayTag::RequestGameplayTag(TEXT("Action.Attack.Basic"));
	}
	
	// Calculate action weights
	float AttackWeight = CalculateAttackWeight();
	float DefenseWeight = CalculateDefenseWeight();
	float AbilityWeight = CalculateAbilityWeight();
	
	// Normalize weights
	float TotalWeight = AttackWeight + DefenseWeight + AbilityWeight;
	if (TotalWeight <= 0.0f)
		TotalWeight = 1.0f;
		
	AttackWeight /= TotalWeight;
	DefenseWeight /= TotalWeight;
	AbilityWeight /= TotalWeight;
	
	// Choose action based on weights
	float RandomValue = FMath::FRandRange(0.0f, 1.0f);
	
	if (RandomValue < AttackWeight)
	{
		// Choose attack type
		if (FMath::FRandRange(0.0f, 1.0f) < ComboLikelihood)
		{
			return FGameplayTag::RequestGameplayTag(TEXT("Action.Attack.Heavy"));
		}
		else
		{
			return FGameplayTag::RequestGameplayTag(TEXT("Action.Attack.Basic"));
		}
	}
	else if (RandomValue < AttackWeight + DefenseWeight)
	{
		// Choose defense type
		if (FMath::FRandRange(0.0f, 1.0f) < 0.5f)
		{
			return FGameplayTag::RequestGameplayTag(TEXT("Action.Block"));
		}
		else
		{
			return FGameplayTag::RequestGameplayTag(TEXT("Action.Dash"));
		}
	}
	else
	{
		// Use ability
		return FGameplayTag::RequestGameplayTag(TEXT("Action.Ability.Soul"));
	}
}

float UAIDifficultyComponent::GetDifficultyRating() const
{
	// Calculate 0-10 difficulty rating
	float Rating = PowerLevel * 1.5f;
	
	// Add behavior modifier
	switch (CurrentBehaviorPreset)
	{
		case EAIBehaviorPreset::Defensive:
			Rating -= 0.5f;
			break;
		case EAIBehaviorPreset::Aggressive:
			Rating += 0.5f;
			break;
		case EAIBehaviorPreset::Berserker:
			Rating += 1.0f;
			break;
		case EAIBehaviorPreset::Tactical:
			Rating += 0.75f;
			break;
		default:
			break;
	}
	
	return FMath::Clamp(Rating, 0.0f, 10.0f);
}

bool UAIDifficultyComponent::IsBerserking() const
{
	if (!OwnerCharacter)
		return false;
		
	if (UHealthComponent* HealthComp = OwnerCharacter->GetHealthComponent())
	{
		float HealthPercent = HealthComp->GetCurrentHealth() / HealthComp->GetMaxHealth();
		return HealthPercent <= BerserkerHealthThreshold;
	}
	
	return false;
}

void UAIDifficultyComponent::ApplyPowerLevelScaling(int32 InPowerLevel)
{
	// Calculate stat modifiers based on power level
	DamageMultiplier = 1.0f + (DamageScalingPerLevel * (InPowerLevel - 1));
	HealthMultiplier = 1.0f + (HealthScalingPerLevel * (InPowerLevel - 1));
	AttackSpeedModifier = 1.0f + (SpeedScalingPerLevel * (InPowerLevel - 1));
	MovementSpeedModifier = 1.0f + (SpeedScalingPerLevel * 0.5f * (InPowerLevel - 1));
	PoiseDamageMultiplier = 1.0f + (0.1f * (InPowerLevel - 1));
	
	// Reaction time gets faster (lower value) as power increases
	ReactionTimeModifier = 1.0f - (0.1f * (InPowerLevel - 1));
	ReactionTimeModifier = FMath::Clamp(ReactionTimeModifier, 0.3f, 1.0f);
	
	// Behavior modifiers scale with power
	AggressionLevel = 0.3f + (0.1f * InPowerLevel);
	DefensePriority = 0.5f - (0.05f * InPowerLevel);
	ComboLikelihood = 0.2f + (0.1f * InPowerLevel);
	AbilityUsageFrequency = 0.1f + (0.1f * InPowerLevel);
	
	// Clamp values
	AggressionLevel = FMath::Clamp(AggressionLevel, 0.0f, 1.0f);
	DefensePriority = FMath::Clamp(DefensePriority, 0.0f, 1.0f);
	ComboLikelihood = FMath::Clamp(ComboLikelihood, 0.0f, 1.0f);
	AbilityUsageFrequency = FMath::Clamp(AbilityUsageFrequency, 0.0f, 1.0f);
	
	UE_LOG(LogTemp, Log, TEXT("Power scaling applied - Damage: x%.2f, Health: x%.2f, Speed: x%.2f"), 
		DamageMultiplier, HealthMultiplier, AttackSpeedModifier);
}

void UAIDifficultyComponent::UpdateBehaviorFromPreset()
{
	switch (CurrentBehaviorPreset)
	{
		case EAIBehaviorPreset::Defensive:
			AggressionLevel = 0.2f;
			DefensePriority = 0.8f;
			ComboLikelihood = 0.1f;
			AbilityUsageFrequency = 0.1f;
			break;
			
		case EAIBehaviorPreset::Balanced:
			AggressionLevel = 0.5f;
			DefensePriority = 0.5f;
			ComboLikelihood = 0.3f;
			AbilityUsageFrequency = 0.2f;
			break;
			
		case EAIBehaviorPreset::Aggressive:
			AggressionLevel = 0.8f;
			DefensePriority = 0.2f;
			ComboLikelihood = 0.5f;
			AbilityUsageFrequency = 0.3f;
			break;
			
		case EAIBehaviorPreset::Berserker:
			AggressionLevel = 1.0f;
			DefensePriority = 0.0f;
			ComboLikelihood = 0.8f;
			AbilityUsageFrequency = 0.5f;
			break;
			
		case EAIBehaviorPreset::Tactical:
			AggressionLevel = 0.4f;
			DefensePriority = 0.4f;
			ComboLikelihood = 0.4f;
			AbilityUsageFrequency = 0.6f;
			break;
	}
}

void UAIDifficultyComponent::CounterDefensivePlayer()
{
	UE_LOG(LogTemp, Log, TEXT("Adapting to counter defensive player"));
	
	// Increase aggression to break through defenses
	AggressionLevel = FMath::Min(AggressionLevel + 0.2f, 1.0f);
	
	// More heavy attacks to break guards
	ComboLikelihood = FMath::Min(ComboLikelihood + 0.2f, 1.0f);
	
	// Use more abilities to bypass blocks
	AbilityUsageFrequency = FMath::Min(AbilityUsageFrequency + 0.1f, 1.0f);
}

void UAIDifficultyComponent::CounterAggressivePlayer()
{
	UE_LOG(LogTemp, Log, TEXT("Adapting to counter aggressive player"));
	
	// Increase defense to punish reckless attacks
	DefensePriority = FMath::Min(DefensePriority + 0.2f, 1.0f);
	
	// Prepare for counter-attacks
	AggressionLevel = FMath::Max(AggressionLevel - 0.1f, 0.3f);
	
	// Focus on parrying and countering
	// This would interface with the AI behavior tree
}

void UAIDifficultyComponent::CounterEvasivePlayer()
{
	UE_LOG(LogTemp, Log, TEXT("Adapting to counter evasive player"));
	
	// Use more area attacks
	AbilityUsageFrequency = FMath::Min(AbilityUsageFrequency + 0.2f, 1.0f);
	
	// Predict dodge patterns
	ReactionTimeModifier = FMath::Max(ReactionTimeModifier - 0.1f, 0.3f);
	
	// Mix up attack timing
	ComboLikelihood = 0.5f; // Balanced to be unpredictable
}

float UAIDifficultyComponent::CalculateAttackWeight() const
{
	float Weight = AggressionLevel;
	
	// Modify based on berserker state
	if (IsBerserking())
	{
		Weight += 0.3f;
	}
	
	// Modify based on player's last action
	if (PlayerPattern.LastPlayerAction.ToString().Contains(TEXT("Block")))
	{
		Weight += 0.1f; // More likely to attack if player is blocking
	}
	
	return FMath::Clamp(Weight, 0.0f, 1.0f);
}

float UAIDifficultyComponent::CalculateDefenseWeight() const
{
	float Weight = DefensePriority;
	
	// Less defense when berserking
	if (IsBerserking())
	{
		Weight -= 0.5f;
	}
	
	// More defense if player is attacking
	if (PlayerPattern.LastPlayerAction.ToString().Contains(TEXT("Attack")))
	{
		Weight += 0.2f;
	}
	
	return FMath::Clamp(Weight, 0.0f, 1.0f);
}

float UAIDifficultyComponent::CalculateAbilityWeight() const
{
	float Weight = AbilityUsageFrequency;
	
	// More abilities when tactical
	if (CurrentBehaviorPreset == EAIBehaviorPreset::Tactical)
	{
		Weight += 0.2f;
	}
	
	// Use abilities to counter evasive players
	if (PlayerPattern.MostUsedAction.ToString().Contains(TEXT("Evasive")))
	{
		Weight += 0.1f;
	}
	
	return FMath::Clamp(Weight, 0.0f, 1.0f);
}