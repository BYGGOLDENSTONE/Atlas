#include "SaveManagerSubsystem.h"
#include "Atlas/Core/AtlasSaveGame.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void USaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("SaveManagerSubsystem initialized"));
	
	// Try to load existing save
	if (HasSaveGame())
	{
		LoadGame();
	}
	else
	{
		// Create new save game
		CreateNewSaveGame();
	}
	
	// Start auto-save timer if enabled
	if (bAutoSaveEnabled && AutoSaveInterval > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				AutoSave();
			}),
			AutoSaveInterval,
			true
		);
	}
}

void USaveManagerSubsystem::Deinitialize()
{
	// Save before shutting down
	if (CurrentSaveGame)
	{
		SaveGame();
	}
	
	// Clear auto-save timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
	
	Super::Deinitialize();
}

bool USaveManagerSubsystem::SaveGame()
{
	if (bIsSaving)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save already in progress"));
		return false;
	}
	
	if (!CurrentSaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("No save game object to save"));
		CreateNewSaveGame();
	}
	
	bIsSaving = true;
	
	// Update metadata
	UpdateSaveMetadata();
	
	// Perform save
	bool bSuccess = PerformSave();
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Game saved successfully"));
		OnSaveGameSaved.Broadcast();
	}
	else
	{
		FText ErrorMsg = FText::FromString(TEXT("Failed to save game"));
		HandleSaveError(ErrorMsg);
	}
	
	bIsSaving = false;
	return bSuccess;
}

bool USaveManagerSubsystem::LoadGame()
{
	if (bIsLoading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Load already in progress"));
		return false;
	}
	
	bIsLoading = true;
	
	// Perform load
	bool bSuccess = PerformLoad();
	
	if (bSuccess && CurrentSaveGame)
	{
		// Validate loaded data
		if (!CurrentSaveGame->ValidateSaveData())
		{
			UE_LOG(LogTemp, Warning, TEXT("Save data validation failed"));
			CreateNewSaveGame();
			bSuccess = false;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Game loaded successfully"));
			OnSaveGameLoaded.Broadcast();
		}
	}
	else
	{
		FText ErrorMsg = FText::FromString(TEXT("Failed to load save game"));
		OnSaveGameFailed.Broadcast(ErrorMsg);
		CreateNewSaveGame();
	}
	
	bIsLoading = false;
	return bSuccess;
}

bool USaveManagerSubsystem::AutoSave()
{
	UE_LOG(LogTemp, Log, TEXT("Auto-saving..."));
	return SaveGame();
}

bool USaveManagerSubsystem::DeleteSaveGame()
{
	bool bDeleted = UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
	
	if (bDeleted)
	{
		UE_LOG(LogTemp, Log, TEXT("Save game deleted"));
		CreateNewSaveGame();
	}
	
	return bDeleted;
}

bool USaveManagerSubsystem::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

bool USaveManagerSubsystem::CanResumeRun() const
{
	return CurrentSaveGame && CurrentSaveGame->bHasRunInProgress;
}

void USaveManagerSubsystem::SaveRewards(USlotManagerComponent* SlotManager)
{
	if (!CurrentSaveGame || !SlotManager)
		return;
		
	// Clear existing rewards
	CurrentSaveGame->EquippedRewards.Empty();
	
	// Get all equipped rewards
	TArray<FEquippedReward> EquippedRewards = SlotManager->GetAllEquippedRewards();
	CurrentSaveGame->EquippedRewards = EquippedRewards;
	
	UE_LOG(LogTemp, Log, TEXT("Saved %d equipped rewards"), EquippedRewards.Num());
	
	// Auto-save after rewards change
	if (bAutoSaveEnabled)
	{
		SaveGame();
	}
}

void USaveManagerSubsystem::LoadRewards(USlotManagerComponent* SlotManager)
{
	if (!CurrentSaveGame || !SlotManager)
		return;
		
	// Clear current rewards
	SlotManager->ClearAllRewards();
	
	// Load saved rewards
	for (const FEquippedReward& SavedReward : CurrentSaveGame->EquippedRewards)
	{
		if (SavedReward.RewardData && SavedReward.SlotIndex >= 0)
		{
			// Equip the reward
			if (SlotManager->EquipReward(SavedReward.RewardData, SavedReward.SlotIndex))
			{
				// Set stack level if greater than 1
				for (int32 i = 1; i < SavedReward.StackLevel; i++)
				{
					SlotManager->EnhanceReward(SavedReward.RewardData->RewardTag);
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Loaded %d rewards"), CurrentSaveGame->EquippedRewards.Num());
}

TArray<FEquippedReward> USaveManagerSubsystem::GetSavedRewards() const
{
	if (CurrentSaveGame)
	{
		return CurrentSaveGame->EquippedRewards;
	}
	return TArray<FEquippedReward>();
}

void USaveManagerSubsystem::UnlockReward(FGameplayTag RewardTag)
{
	if (!CurrentSaveGame)
		return;
		
	if (!CurrentSaveGame->UnlockedRewards.Contains(RewardTag))
	{
		CurrentSaveGame->UnlockedRewards.Add(RewardTag, 1);
		UE_LOG(LogTemp, Log, TEXT("Unlocked reward: %s"), *RewardTag.ToString());
	}
	else
	{
		// Increment unlock count (for tracking how many times found)
		CurrentSaveGame->UnlockedRewards[RewardTag]++;
	}
}

bool USaveManagerSubsystem::IsRewardUnlocked(FGameplayTag RewardTag) const
{
	if (!CurrentSaveGame)
		return false;
		
	return CurrentSaveGame->UnlockedRewards.Contains(RewardTag);
}

void USaveManagerSubsystem::SaveRunProgress(const FRunProgressData& RunData)
{
	if (!CurrentSaveGame)
		return;
		
	// Save run progress
	CurrentSaveGame->CurrentRunLevel = RunData.CurrentLevel;
	CurrentSaveGame->CurrentHealth = RunData.PlayerHealth;
	CurrentSaveGame->StationIntegrity = RunData.StationIntegrity;
	CurrentSaveGame->CompletedRoomIDs = RunData.CompletedRoomIDs;
	CurrentSaveGame->bHasRunInProgress = true;
	
	// Update statistics
	CurrentSaveGame->UpdateStatistics(RunData);
	
	UE_LOG(LogTemp, Log, TEXT("Saved run progress - Level %d"), RunData.CurrentLevel);
	
	// Auto-save
	if (bAutoSaveEnabled)
	{
		SaveGame();
	}
}

bool USaveManagerSubsystem::LoadRunProgress(FRunProgressData& OutRunData)
{
	if (!CurrentSaveGame || !CurrentSaveGame->bHasRunInProgress)
		return false;
		
	// Load run progress
	OutRunData.CurrentLevel = CurrentSaveGame->CurrentRunLevel;
	OutRunData.PlayerHealth = CurrentSaveGame->CurrentHealth;
	OutRunData.StationIntegrity = CurrentSaveGame->StationIntegrity;
	OutRunData.CompletedRoomIDs = CurrentSaveGame->CompletedRoomIDs;
	
	UE_LOG(LogTemp, Log, TEXT("Loaded run progress - Level %d"), OutRunData.CurrentLevel);
	return true;
}

void USaveManagerSubsystem::ClearCurrentRun()
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->ClearCurrentRun();
		SaveGame();
	}
}

void USaveManagerSubsystem::UpdateRunStatistics(const FRunProgressData& RunData)
{
	if (CurrentSaveGame)
	{
		CurrentSaveGame->UpdateStatistics(RunData);
		SaveGame();
	}
}

int32 USaveManagerSubsystem::GetTotalRunsCompleted() const
{
	return CurrentSaveGame ? CurrentSaveGame->TotalRunsCompleted : 0;
}

int32 USaveManagerSubsystem::GetHighestRoomReached() const
{
	return CurrentSaveGame ? CurrentSaveGame->HighestRoomReached : 0;
}

int32 USaveManagerSubsystem::GetTotalEnemiesDefeated() const
{
	return CurrentSaveGame ? CurrentSaveGame->TotalEnemiesDefeated : 0;
}

int32 USaveManagerSubsystem::GetBestRunRecord() const
{
	return CurrentSaveGame ? CurrentSaveGame->BestRunRooms : 0;
}

void USaveManagerSubsystem::IncrementStat(FName StatName, int32 Amount)
{
	if (!CurrentSaveGame)
		return;
		
	// Update specific stats based on name
	if (StatName == TEXT("EnemiesDefeated"))
	{
		CurrentSaveGame->TotalEnemiesDefeated += Amount;
	}
	else if (StatName == TEXT("PerfectParries"))
	{
		CurrentSaveGame->PerfectParries += Amount;
	}
	else if (StatName == TEXT("SuccessfulBlocks"))
	{
		CurrentSaveGame->SuccessfulBlocks += Amount;
	}
	else if (StatName == TEXT("Deaths"))
	{
		CurrentSaveGame->TotalDeaths += Amount;
	}
	else if (StatName == TEXT("StationDestructions"))
	{
		CurrentSaveGame->StationDestructions += Amount;
	}
}

void USaveManagerSubsystem::SaveSettings(float MasterVol, float MusicVol, float SFXVol)
{
	if (!CurrentSaveGame)
		return;
		
	CurrentSaveGame->MasterVolume = MasterVol;
	CurrentSaveGame->MusicVolume = MusicVol;
	CurrentSaveGame->SFXVolume = SFXVol;
	
	SaveGame();
}

void USaveManagerSubsystem::LoadSettings(float& OutMasterVol, float& OutMusicVol, float& OutSFXVol)
{
	if (!CurrentSaveGame)
	{
		OutMasterVol = 1.0f;
		OutMusicVol = 0.8f;
		OutSFXVol = 1.0f;
		return;
	}
	
	OutMasterVol = CurrentSaveGame->MasterVolume;
	OutMusicVol = CurrentSaveGame->MusicVolume;
	OutSFXVol = CurrentSaveGame->SFXVolume;
}

void USaveManagerSubsystem::UnlockAchievement(FName AchievementName)
{
	if (!CurrentSaveGame)
		return;
		
	if (!CurrentSaveGame->AchievementFlags.Contains(AchievementName))
	{
		CurrentSaveGame->AchievementFlags.Add(AchievementName, true);
		UE_LOG(LogTemp, Log, TEXT("Achievement unlocked: %s"), *AchievementName.ToString());
		
		// TODO: Trigger platform-specific achievement
		
		SaveGame();
	}
}

bool USaveManagerSubsystem::IsAchievementUnlocked(FName AchievementName) const
{
	if (!CurrentSaveGame)
		return false;
		
	const bool* Value = CurrentSaveGame->AchievementFlags.Find(AchievementName);
	return Value && *Value;
}

void USaveManagerSubsystem::CreateNewSaveGame()
{
	CurrentSaveGame = Cast<UAtlasSaveGame>(UGameplayStatics::CreateSaveGameObject(UAtlasSaveGame::StaticClass()));
	
	if (CurrentSaveGame)
	{
		UE_LOG(LogTemp, Log, TEXT("Created new save game"));
		
		// Initialize with defaults
		CurrentSaveGame->LastSaveTime = FDateTime::Now();
		CurrentSaveGame->SaveGameVersion = 1;
		
		// Perform initial save
		SaveGame();
	}
}

bool USaveManagerSubsystem::ValidateSaveGame() const
{
	return CurrentSaveGame && CurrentSaveGame->ValidateSaveData();
}

FString USaveManagerSubsystem::ExportSaveData()
{
	if (!CurrentSaveGame)
		return FString();
		
	// Serialize save game to bytes
	FBufferArchive Archive;
	Archive.ArIsSaveGame = true;
	CurrentSaveGame->Serialize(Archive);
	
	// Convert to base64 string for easy transport
	FString SaveDataString = FBase64::Encode(Archive);
	
	UE_LOG(LogTemp, Log, TEXT("Exported save data (%d bytes)"), Archive.Num());
	return SaveDataString;
}

bool USaveManagerSubsystem::ImportSaveData(const FString& SaveDataString)
{
	// Decode from base64
	TArray<uint8> SaveDataBytes;
	if (!FBase64::Decode(SaveDataString, SaveDataBytes))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decode save data"));
		return false;
	}
	
	// Deserialize into save game object
	FMemoryReader Archive(SaveDataBytes);
	Archive.ArIsSaveGame = true;
	
	UAtlasSaveGame* ImportedSave = Cast<UAtlasSaveGame>(UGameplayStatics::CreateSaveGameObject(UAtlasSaveGame::StaticClass()));
	if (ImportedSave)
	{
		ImportedSave->Serialize(Archive);
		
		// Validate imported data
		if (ImportedSave->ValidateSaveData())
		{
			CurrentSaveGame = ImportedSave;
			UE_LOG(LogTemp, Log, TEXT("Successfully imported save data"));
			OnSaveGameLoaded.Broadcast();
			return true;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("Failed to import save data"));
	return false;
}

bool USaveManagerSubsystem::PerformSave()
{
	if (!CurrentSaveGame)
		return false;
		
	return UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, UserIndex);
}

bool USaveManagerSubsystem::PerformLoad()
{
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex);
	
	if (LoadedGame)
	{
		CurrentSaveGame = Cast<UAtlasSaveGame>(LoadedGame);
		
		// Check version and migrate if needed
		if (CurrentSaveGame && CurrentSaveGame->SaveGameVersion < 1)
		{
			MigrateSaveData(CurrentSaveGame->SaveGameVersion);
		}
		
		return CurrentSaveGame != nullptr;
	}
	
	return false;
}

void USaveManagerSubsystem::UpdateSaveMetadata()
{
	if (!CurrentSaveGame)
		return;
		
	// Update timestamp
	CurrentSaveGame->LastSaveTime = FDateTime::Now();
	
	// Calculate and store checksum
	CurrentSaveGame->SaveChecksum = CurrentSaveGame->CalculateChecksum();
}

void USaveManagerSubsystem::HandleSaveError(const FText& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("Save error: %s"), *ErrorMessage.ToString());
	OnSaveGameFailed.Broadcast(ErrorMessage);
}

void USaveManagerSubsystem::MigrateSaveData(int32 OldVersion)
{
	UE_LOG(LogTemp, Log, TEXT("Migrating save data from version %d to %d"), 
		OldVersion, CurrentSaveGame->SaveGameVersion);
	
	// Handle migration based on version differences
	// For now, we only have version 1
	
	// Update to current version
	CurrentSaveGame->SaveGameVersion = 1;
}