#include "AtlasSaveGame.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"

UAtlasSaveGame::UAtlasSaveGame()
{
	// Set initial save time
	LastSaveTime = FDateTime::Now();
	
	// Initialize with current version
	SaveGameVersion = 1;
}

void UAtlasSaveGame::UpdateStatistics(const FRunProgressData& RunData)
{
	// Update enemy statistics
	TotalEnemiesDefeated += RunData.TotalEnemiesDefeated;
	TotalDamageDealt += RunData.TotalDamageDealt;
	PerfectParries += RunData.PerfectParries;
	
	// Update best run if applicable
	int32 RoomsCompleted = RunData.CompletedRoomIDs.Num();
	if (RoomsCompleted > BestRunRooms)
	{
		BestRunRooms = RoomsCompleted;
	}
	
	// Update highest room reached
	if (RunData.CurrentLevel > HighestRoomReached)
	{
		HighestRoomReached = RunData.CurrentLevel;
	}
	
	// Check for run completion
	if (RoomsCompleted >= 5)
	{
		TotalRunsCompleted++;
		
		// Update fastest time if this was a complete run
		// Note: CurrentRunTime should be set when saving
		if (CurrentRunTime > 0.0f && CurrentRunTime < FastestRunTime)
		{
			FastestRunTime = CurrentRunTime;
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Statistics updated - Total enemies defeated: %d, Total runs: %d"), 
		TotalEnemiesDefeated, TotalRunsCompleted);
}

void UAtlasSaveGame::ClearCurrentRun()
{
	// Clear current run data but keep persistent progression
	CurrentRunLevel = 0;
	CurrentHealth = 100.0f;
	StationIntegrity = 100.0f;
	CompletedRoomIDs.Empty();
	CurrentRunRewards.Empty();
	CurrentRunTime = 0.0f;
	bHasRunInProgress = false;
	
	UE_LOG(LogTemp, Log, TEXT("Current run data cleared"));
}

bool UAtlasSaveGame::ValidateSaveData() const
{
	// Check version compatibility
	if (SaveGameVersion > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save game version %d is newer than supported version"), SaveGameVersion);
		return false;
	}
	
	// Validate checksum
	uint32 ExpectedChecksum = CalculateChecksum();
	if (SaveChecksum != 0 && SaveChecksum != ExpectedChecksum)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save game checksum mismatch! Expected: %u, Got: %u"), 
			ExpectedChecksum, SaveChecksum);
		return false;
	}
	
	// Validate data ranges
	if (CurrentHealth < 0.0f || CurrentHealth > 1000.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid health value: %f"), CurrentHealth);
		return false;
	}
	
	if (StationIntegrity < 0.0f || StationIntegrity > 100.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid station integrity: %f"), StationIntegrity);
		return false;
	}
	
	if (CurrentRunLevel < 0 || CurrentRunLevel > 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid run level: %d"), CurrentRunLevel);
		return false;
	}
	
	// Check for data corruption in arrays
	if (EquippedRewards.Num() > 100) // Sanity check
	{
		UE_LOG(LogTemp, Warning, TEXT("Suspicious number of equipped rewards: %d"), EquippedRewards.Num());
		return false;
	}
	
	return true;
}

uint32 UAtlasSaveGame::CalculateChecksum() const
{
	// Simple checksum calculation based on key data
	// In production, use a proper hashing algorithm
	uint32 Checksum = 0;
	
	// Include version
	Checksum ^= SaveGameVersion;
	
	// Include progression data
	Checksum ^= TotalRunsCompleted;
	Checksum ^= HighestRoomReached;
	Checksum ^= TotalEnemiesDefeated;
	Checksum ^= static_cast<uint32>(TotalDamageDealt);
	
	// Include equipped rewards count
	Checksum ^= EquippedRewards.Num();
	
	// Include some reward data
	for (const FEquippedReward& Reward : EquippedRewards)
	{
		if (Reward.RewardData)
		{
			Checksum ^= GetTypeHash(Reward.RewardData->RewardTag.ToString());
			Checksum ^= Reward.StackLevel;
		}
	}
	
	// Include unlocked rewards
	Checksum ^= UnlockedRewards.Num();
	
	// Include statistics
	Checksum ^= TotalDeaths;
	Checksum ^= StationDestructions;
	
	return Checksum;
}

FString UAtlasSaveGame::GetFormattedPlayTime() const
{
	// Format current run time or total play time
	float TimeInSeconds = CurrentRunTime;
	
	int32 Hours = FMath::FloorToInt(TimeInSeconds / 3600.0f);
	TimeInSeconds -= Hours * 3600.0f;
	
	int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
	TimeInSeconds -= Minutes * 60.0f;
	
	int32 Seconds = FMath::FloorToInt(TimeInSeconds);
	
	if (Hours > 0)
	{
		return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}
}