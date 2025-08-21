#include "RoomDataAsset.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Math/UnrealMathUtility.h"

TArray<URewardDataAsset*> URoomDataAsset::SelectRandomRewards(int32 Count, int32 PlayerLevel) const
{
	TArray<URewardDataAsset*> SelectedRewards;
	
	// Filter eligible rewards based on player level
	TArray<FRewardChoice> EligibleRewards;
	float TotalWeight = 0.0f;
	
	for (const FRewardChoice& Choice : RewardPool)
	{
		if (Choice.Reward && Choice.MinimumLevel <= PlayerLevel)
		{
			EligibleRewards.Add(Choice);
			TotalWeight += Choice.SelectionWeight;
		}
	}
	
	// Add guaranteed reward first if it exists
	if (GuaranteedReward && SelectedRewards.Num() < Count)
	{
		SelectedRewards.Add(GuaranteedReward);
	}
	
	// Select random rewards based on weights
	while (SelectedRewards.Num() < Count && EligibleRewards.Num() > 0)
	{
		float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;
		
		for (int32 i = 0; i < EligibleRewards.Num(); i++)
		{
			CurrentWeight += EligibleRewards[i].SelectionWeight;
			if (RandomValue <= CurrentWeight)
			{
				// Select this reward
				URewardDataAsset* Selected = EligibleRewards[i].Reward;
				
				// Avoid duplicates
				if (!SelectedRewards.Contains(Selected))
				{
					SelectedRewards.Add(Selected);
				}
				
				// Remove from eligible pool to avoid selecting again
				TotalWeight -= EligibleRewards[i].SelectionWeight;
				EligibleRewards.RemoveAt(i);
				break;
			}
		}
	}
	
	// Check for bonus reward
	if (BonusReward && FMath::FRandRange(0.0f, 1.0f) <= BonusRewardChance)
	{
		// Replace a random non-guaranteed reward with bonus, or add if room
		if (SelectedRewards.Num() < Count)
		{
			SelectedRewards.Add(BonusReward);
		}
		else if (SelectedRewards.Num() > 1)
		{
			// Replace a random non-guaranteed reward
			int32 ReplaceIndex = FMath::RandRange(1, SelectedRewards.Num() - 1);
			SelectedRewards[ReplaceIndex] = BonusReward;
		}
	}
	
	return SelectedRewards;
}

FText URoomDataAsset::GetHazardDisplayName() const
{
	switch (EnvironmentalHazard)
	{
		case ERoomHazard::None:
			return FText::FromString(TEXT("Normal Environment"));
			
		case ERoomHazard::LowGravity:
			return FText::FromString(TEXT("Low Gravity"));
			
		case ERoomHazard::ElectricalSurges:
			return FText::FromString(TEXT("Electrical Surges"));
			
		case ERoomHazard::HullBreach:
			return FText::FromString(TEXT("Hull Breach"));
			
		case ERoomHazard::ToxicLeak:
			return FText::FromString(TEXT("Toxic Leak"));
			
		case ERoomHazard::SystemMalfunction:
			return FText::FromString(TEXT("System Malfunction"));
			
		default:
			return FText::FromString(TEXT("Unknown Hazard"));
	}
}