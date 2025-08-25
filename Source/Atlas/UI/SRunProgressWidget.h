#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class URunManagerComponent;
class UHealthComponent;
class UStationIntegrityComponent;
class URoomDataAsset;
enum class ERoomHazard : uint8;

/**
 * Slate widget for displaying run progress, room progression, and vital stats
 */
class ATLAS_API SRunProgressWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRunProgressWidget)
	{}
		SLATE_ARGUMENT(URunManagerComponent*, RunManager)
		SLATE_ARGUMENT(UHealthComponent*, HealthComponent)
		SLATE_ARGUMENT(UStationIntegrityComponent*, IntegrityComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	void UpdateRoomProgress(int32 CurrentRoomIndex, const TArray<URoomDataAsset*>& CompletedRooms);
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	void UpdatePoise(float CurrentPoise, float MaxPoise);
	void UpdateIntegrity(float CurrentIntegrity, float MaxIntegrity);
	void UpdateCurrentRoomInfo(URoomDataAsset* RoomData);
	void SetRoomCompleted(int32 RoomIndex);
	void SetRoomFailed(int32 RoomIndex);

private:
	TSharedRef<SWidget> CreateRoomProgressBar();
	TSharedRef<SWidget> CreateHealthBar();
	TSharedRef<SWidget> CreatePoiseBar();
	TSharedRef<SWidget> CreateIntegrityBar();
	TSharedRef<SWidget> CreateRoomIcon(int32 RoomIndex);
	
	FText GetRoomProgressText() const;
	FText GetHealthText() const;
	FText GetPoiseText() const;
	FText GetIntegrityText() const;
	FText GetCurrentRoomName() const;
	FText GetHazardText(ERoomHazard Hazard) const;
	
	FSlateColor GetHealthBarColor() const;
	FSlateColor GetPoiseBarColor() const;
	FSlateColor GetIntegrityBarColor() const;
	FSlateColor GetRoomIconColor(int32 RoomIndex) const;
	
	TOptional<float> GetHealthPercent() const;
	TOptional<float> GetPoisePercent() const;
	TOptional<float> GetIntegrityPercent() const;

private:
	URunManagerComponent* RunManagerRef;
	UHealthComponent* HealthComponentRef;
	UStationIntegrityComponent* IntegrityComponentRef;
	
	int32 CurrentRoomIndex;
	TArray<bool> RoomCompletionStatus;
	TArray<bool> RoomFailureStatus;
	
	float CurrentHealth;
	float MaxHealth;
	float CurrentPoise;
	float MaxPoise;
	float CurrentIntegrity;
	float MaxIntegrity;
	
	URoomDataAsset* CurrentRoomData;
	
	static constexpr int32 TotalRooms = 5;
	static constexpr float LowHealthThreshold = 0.25f;
	static constexpr float LowIntegrityThreshold = 0.25f;
};