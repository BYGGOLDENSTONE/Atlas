#include "SlotManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Components/ActionManagerComponent.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

USlotManagerComponent::USlotManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize arrays with default size
	EquippedRewards.SetNum(MaxSlots);
}

void USlotManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get owner references
	OwnerCharacter = Cast<AGameCharacterBase>(GetOwner());
	if (OwnerCharacter)
	{
		ActionManagerComponent = OwnerCharacter->GetActionManagerComponent();
	}
	
	// Load rewards from save or apply defaults
	LoadRewardsFromSave();
	
	// If no saved rewards, apply default loadout
	if (GetUsedSlotCount() == 0 && DefaultRewards.Num() > 0)
	{
		ResetToDefaultLoadout();
	}
	
	// Apply initial effects
	UpdateOwnerStats();
	RegisterActiveAbilities();
}

bool USlotManagerComponent::EquipReward(URewardDataAsset* Reward, int32 SlotIndex)
{
	if (!Reward || SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}
	
	// Check if we can equip this reward
	FText FailReason;
	if (!CanEquipReward(Reward, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot equip reward: %s"), *FailReason.ToString());
		return false;
	}
	
	// Check if slot is occupied
	if (EquippedRewards[SlotIndex].RewardData != nullptr)
	{
		// Check if it's the same reward (for stacking)
		if (EquippedRewards[SlotIndex].RewardData->RewardTag == Reward->RewardTag)
		{
			return EnhanceReward(Reward->RewardTag);
		}
		else
		{
			// Slot occupied by different reward
			return false;
		}
	}
	
	// Check if we have enough slots
	if (!HasSlotsForReward(Reward))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough slots for reward (requires %d)"), Reward->SlotCost);
		return false;
	}
	
	// Equip the reward
	FEquippedReward NewReward(Reward, SlotIndex);
	EquippedRewards[SlotIndex] = NewReward;
	RewardStacks.Add(Reward->RewardTag, 1);
	
	// Apply effects
	ApplyRewardEffects(NewReward);
	
	// Broadcast events
	OnRewardEquipped.Broadcast(SlotIndex, Reward);
	OnSlotsChanged.Broadcast();
	
	UE_LOG(LogTemp, Log, TEXT("Equipped reward: %s to slot %d"), *Reward->RewardName.ToString(), SlotIndex);
	return true;
}

bool USlotManagerComponent::RemoveReward(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}
	
	FEquippedReward& Reward = EquippedRewards[SlotIndex];
	if (Reward.RewardData == nullptr)
	{
		return false;
	}
	
	// Store for event
	URewardDataAsset* RemovedReward = Reward.RewardData;
	
	// Remove effects
	RemoveRewardEffects(Reward);
	
	// Clear the slot
	RewardStacks.Remove(Reward.RewardData->RewardTag);
	Reward = FEquippedReward();
	
	// Update stats
	UpdateOwnerStats();
	
	// Broadcast events
	OnRewardRemoved.Broadcast(SlotIndex, RemovedReward);
	OnSlotsChanged.Broadcast();
	
	return true;
}

bool USlotManagerComponent::EnhanceReward(FGameplayTag RewardTag)
{
	FEquippedReward FoundReward = FindRewardByTag(RewardTag);
	if (!FoundReward.RewardData)
	{
		return false;
	}
	
	// Check if can enhance further
	if (FoundReward.StackLevel >= FoundReward.RewardData->MaxStackLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reward already at max stack level"));
		return false;
	}
	
	// Enhance the reward
	int32 SlotIndex = FoundReward.SlotIndex;
	EquippedRewards[SlotIndex].StackLevel++;
	RewardStacks[RewardTag] = EquippedRewards[SlotIndex].StackLevel;
	
	// Update effects
	RemoveRewardEffects(FoundReward);
	ApplyRewardEffects(EquippedRewards[SlotIndex]);
	UpdateOwnerStats();
	
	// Broadcast event
	OnRewardEnhanced.Broadcast(SlotIndex, FoundReward.RewardData, EquippedRewards[SlotIndex].StackLevel);
	
	UE_LOG(LogTemp, Log, TEXT("Enhanced reward %s to level %d"), 
		*FoundReward.RewardData->RewardName.ToString(), EquippedRewards[SlotIndex].StackLevel);
	
	return true;
}

bool USlotManagerComponent::ReplaceReward(int32 SlotIndex, URewardDataAsset* NewReward)
{
	if (!NewReward || SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}
	
	// Remove existing reward
	RemoveReward(SlotIndex);
	
	// Equip new reward
	return EquipReward(NewReward, SlotIndex);
}

bool USlotManagerComponent::SwapRewards(int32 SlotA, int32 SlotB)
{
	if (SlotA < 0 || SlotA >= MaxSlots || SlotB < 0 || SlotB >= MaxSlots)
	{
		return false;
	}
	
	// Swap the rewards
	FEquippedReward Temp = EquippedRewards[SlotA];
	EquippedRewards[SlotA] = EquippedRewards[SlotB];
	EquippedRewards[SlotB] = Temp;
	
	// Update slot indices
	if (EquippedRewards[SlotA].RewardData)
		EquippedRewards[SlotA].SlotIndex = SlotA;
	if (EquippedRewards[SlotB].RewardData)
		EquippedRewards[SlotB].SlotIndex = SlotB;
	
	OnSlotsChanged.Broadcast();
	return true;
}

void USlotManagerComponent::ClearAllRewards()
{
	for (int32 i = 0; i < MaxSlots; i++)
	{
		RemoveReward(i);
	}
}

URewardDataAsset* USlotManagerComponent::GetRewardInSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= EquippedRewards.Num())
	{
		return nullptr;
	}
	return EquippedRewards[SlotIndex].RewardData;
}

int32 USlotManagerComponent::GetRewardStackLevel(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= EquippedRewards.Num())
	{
		return 0;
	}
	return EquippedRewards[SlotIndex].StackLevel;
}

FEquippedReward USlotManagerComponent::FindRewardByTag(FGameplayTag RewardTag) const
{
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (Reward.RewardData && Reward.RewardData->RewardTag == RewardTag)
		{
			return Reward;
		}
	}
	return FEquippedReward();
}

bool USlotManagerComponent::IsRewardEquipped(FGameplayTag RewardTag) const
{
	return FindRewardByTag(RewardTag).RewardData != nullptr;
}

int32 USlotManagerComponent::GetUsedSlotCount() const
{
	int32 Count = 0;
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (Reward.RewardData)
		{
			Count += Reward.RewardData->SlotCost;
		}
	}
	return Count;
}

int32 USlotManagerComponent::GetAvailableSlotCount() const
{
	return MaxSlots - GetUsedSlotCount();
}

bool USlotManagerComponent::HasSlotsForReward(URewardDataAsset* Reward) const
{
	return Reward && GetAvailableSlotCount() >= Reward->SlotCost;
}

TArray<FEquippedReward> USlotManagerComponent::GetAllEquippedRewards() const
{
	TArray<FEquippedReward> Result;
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (Reward.RewardData)
		{
			Result.Add(Reward);
		}
	}
	return Result;
}

TArray<FEquippedReward> USlotManagerComponent::GetRewardsByCategory(ERewardCategory Category) const
{
	TArray<FEquippedReward> Result;
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (Reward.RewardData && Reward.RewardData->Category == Category)
		{
			Result.Add(Reward);
		}
	}
	return Result;
}

float USlotManagerComponent::CalculateTotalStatModifier(FName StatName) const
{
	float Total = 0.0f;
	
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (!Reward.RewardData || Reward.RewardData->Category != ERewardCategory::PassiveStats)
			continue;
			
		if (const float* ModValue = Reward.RewardData->StatModifiers.Find(StatName))
		{
			float StackMultiplier = Reward.RewardData->GetStackMultiplier(Reward.StackLevel);
			Total += (*ModValue * StackMultiplier);
		}
	}
	
	return Total;
}

TMap<FName, float> USlotManagerComponent::GetAllStatModifiers() const
{
	TMap<FName, float> AllModifiers;
	
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (!Reward.RewardData || Reward.RewardData->Category != ERewardCategory::PassiveStats)
			continue;
			
		float StackMultiplier = Reward.RewardData->GetStackMultiplier(Reward.StackLevel);
		
		for (const auto& Pair : Reward.RewardData->StatModifiers)
		{
			if (AllModifiers.Contains(Pair.Key))
			{
				AllModifiers[Pair.Key] += (Pair.Value * StackMultiplier);
			}
			else
			{
				AllModifiers.Add(Pair.Key, Pair.Value * StackMultiplier);
			}
		}
	}
	
	return AllModifiers;
}

void USlotManagerComponent::SaveRewardsForNextRun()
{
	// TODO: Implement save system
	// This would save to a save game object or persistent data
	UE_LOG(LogTemp, Log, TEXT("Saving rewards for next run..."));
}

void USlotManagerComponent::LoadRewardsFromSave()
{
	// TODO: Implement load system
	// This would load from a save game object or persistent data
	UE_LOG(LogTemp, Log, TEXT("Loading rewards from save..."));
}

void USlotManagerComponent::ResetToDefaultLoadout()
{
	ClearAllRewards();
	
	int32 SlotIndex = 0;
	for (URewardDataAsset* DefaultReward : DefaultRewards)
	{
		if (DefaultReward && SlotIndex < MaxSlots)
		{
			EquipReward(DefaultReward, SlotIndex);
			SlotIndex++;
		}
	}
}

bool USlotManagerComponent::CanEquipReward(URewardDataAsset* Reward, FText& OutReason) const
{
	if (!Reward)
	{
		OutReason = FText::FromString(TEXT("Invalid reward"));
		return false;
	}
	
	// Check slot availability
	if (!HasSlotsForReward(Reward))
	{
		OutReason = FText::Format(
			FText::FromString(TEXT("Not enough slots (requires {0}, have {1})")),
			FText::AsNumber(Reward->SlotCost),
			FText::AsNumber(GetAvailableSlotCount())
		);
		return false;
	}
	
	// Check for incompatible rewards
	TArray<FGameplayTag> Incompatible = GetIncompatibleRewards(Reward);
	if (Incompatible.Num() > 0)
	{
		OutReason = FText::FromString(TEXT("Incompatible with equipped rewards"));
		return false;
	}
	
	// Check requirements
	// TODO: Check player level, required tags, etc.
	
	OutReason = FText::GetEmpty();
	return true;
}

TArray<FGameplayTag> USlotManagerComponent::GetIncompatibleRewards(URewardDataAsset* Reward) const
{
	TArray<FGameplayTag> Incompatible;
	
	if (!Reward)
		return Incompatible;
		
	for (const FEquippedReward& Equipped : EquippedRewards)
	{
		if (!Equipped.RewardData)
			continue;
			
		// Check if this equipped reward is incompatible
		if (Reward->IncompatibleRewards.Contains(Equipped.RewardData->RewardTag))
		{
			Incompatible.Add(Equipped.RewardData->RewardTag);
		}
		
		// Check reverse - if equipped reward lists this one as incompatible
		if (Equipped.RewardData->IncompatibleRewards.Contains(Reward->RewardTag))
		{
			Incompatible.Add(Equipped.RewardData->RewardTag);
		}
	}
	
	return Incompatible;
}

void USlotManagerComponent::ApplyRewardEffects(const FEquippedReward& Reward)
{
	if (!Reward.RewardData)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Applying effects for reward: %s"), *Reward.RewardData->RewardName.ToString());
	
	// Apply based on category
	switch (Reward.RewardData->Category)
	{
		case ERewardCategory::Defense:
		case ERewardCategory::Offense:
			// Register active ability with ActionManager
			if (Reward.RewardData->LinkedAction && ActionManagerComponent)
			{
				// TODO: Assign to appropriate slot or replace existing
			}
			break;
			
		case ERewardCategory::PassiveStats:
			// Stats are applied through UpdateOwnerStats()
			break;
			
		case ERewardCategory::PassiveAbility:
			// Apply passive ability effects
			// TODO: Implement passive ability system
			break;
			
		case ERewardCategory::Interactable:
			// Enable interactable abilities
			// TODO: Implement interactable system
			break;
	}
	
	// Spawn persistent VFX if configured
	if (Reward.RewardData->PersistentVFX && OwnerCharacter)
	{
		// TODO: Spawn and attach VFX to character
	}
}

void USlotManagerComponent::RemoveRewardEffects(const FEquippedReward& Reward)
{
	if (!Reward.RewardData)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Removing effects for reward: %s"), *Reward.RewardData->RewardName.ToString());
	
	// Remove based on category
	switch (Reward.RewardData->Category)
	{
		case ERewardCategory::Defense:
		case ERewardCategory::Offense:
			// Unregister active ability
			if (Reward.RewardData->LinkedAction && ActionManagerComponent)
			{
				// TODO: Remove from ActionManager
			}
			break;
			
		case ERewardCategory::PassiveStats:
			// Stats will be recalculated in UpdateOwnerStats()
			break;
			
		case ERewardCategory::PassiveAbility:
			// Remove passive ability effects
			// TODO: Implement passive ability system
			break;
			
		case ERewardCategory::Interactable:
			// Disable interactable abilities
			// TODO: Implement interactable system
			break;
	}
	
	// Remove persistent VFX
	// TODO: Find and destroy attached VFX
}

void USlotManagerComponent::UpdateOwnerStats()
{
	if (!OwnerCharacter)
		return;
		
	// Get all stat modifiers
	TMap<FName, float> AllMods = GetAllStatModifiers();
	
	// Apply to health component
	if (UHealthComponent* HealthComp = OwnerCharacter->GetHealthComponent())
	{
		if (const float* MaxHealthMod = AllMods.Find(TEXT("MaxHealth")))
		{
			// TODO: Apply max health modifier
			UE_LOG(LogTemp, Log, TEXT("Max Health Modifier: +%f"), *MaxHealthMod);
		}
		
		if (const float* PoiseMaxMod = AllMods.Find(TEXT("PoiseMax")))
		{
			// TODO: Apply max poise modifier
			UE_LOG(LogTemp, Log, TEXT("Max Poise Modifier: +%f"), *PoiseMaxMod);
		}
	}
	
	// Apply movement speed
	if (ACharacter* Character = Cast<ACharacter>(OwnerCharacter))
	{
		if (const float* MoveSpeedMod = AllMods.Find(TEXT("MoveSpeed")))
		{
			// TODO: Apply movement speed modifier
			UE_LOG(LogTemp, Log, TEXT("Move Speed Modifier: x%f"), 1.0f + *MoveSpeedMod);
		}
	}
	
	// Log all active modifiers
	for (const auto& Pair : AllMods)
	{
		UE_LOG(LogTemp, Log, TEXT("Active Stat Modifier - %s: %f"), *Pair.Key.ToString(), Pair.Value);
	}
}

void USlotManagerComponent::RegisterActiveAbilities()
{
	if (!ActionManagerComponent)
		return;
		
	// Register all active ability rewards
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (!Reward.RewardData)
			continue;
			
		if ((Reward.RewardData->Category == ERewardCategory::Defense || 
		     Reward.RewardData->Category == ERewardCategory::Offense) &&
		    Reward.RewardData->LinkedAction)
		{
			// TODO: Register with ActionManager
			UE_LOG(LogTemp, Log, TEXT("Registering active ability: %s"), 
				*Reward.RewardData->RewardName.ToString());
		}
	}
}

void USlotManagerComponent::ValidateEquippedRewards()
{
	// Check all equipped rewards are still valid
	for (int32 i = 0; i < EquippedRewards.Num(); i++)
	{
		if (EquippedRewards[i].RewardData)
		{
			FText FailReason;
			if (!CanEquipReward(EquippedRewards[i].RewardData, FailReason))
			{
				UE_LOG(LogTemp, Warning, TEXT("Reward %s no longer valid: %s"), 
					*EquippedRewards[i].RewardData->RewardName.ToString(), 
					*FailReason.ToString());
				RemoveReward(i);
			}
		}
	}
}