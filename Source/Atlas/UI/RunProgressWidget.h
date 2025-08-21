#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "RunProgressWidget.generated.h"

// Forward declarations
class URunManagerComponent;
class UHealthComponent;
class UStationIntegrityComponent;
class UProgressBar;
class UTextBlock;
class UHorizontalBox;
class UImage;
class UVerticalBox;

/**
 * Room icon state enum
 */
UENUM(BlueprintType)
enum class ERoomIconState : uint8
{
	Locked,
	Current,
	Completed,
	Failed
};

/**
 * Individual room icon widget
 */
UCLASS()
class ATLAS_API URoomIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS
	// ========================================
	
	/** Room icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* RoomIcon;
	
	/** Room number text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* RoomNumberText;
	
	/** Completed checkmark */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* CompletedCheckmark;
	
	/** Current indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* CurrentIndicator;
	
	/** Locked icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* LockedIcon;
	
	/** Failed X mark */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* FailedMark;
	
	// ========================================
	// ROOM STATE
	// ========================================
	
	/** Current state of this room */
	UPROPERTY(BlueprintReadOnly)
	ERoomIconState RoomState = ERoomIconState::Locked;
	
	/** Room number (1-5) */
	UPROPERTY(BlueprintReadOnly)
	int32 RoomNumber = 1;
	
	/** Room data */
	UPROPERTY(BlueprintReadOnly)
	URoomDataAsset* RoomData = nullptr;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize room icon */
	UFUNCTION(BlueprintCallable, Category = "Room Icon")
	void InitializeRoomIcon(int32 Number, URoomDataAsset* Data = nullptr);
	
	/** Set room state */
	UFUNCTION(BlueprintCallable, Category = "Room Icon")
	void SetRoomState(ERoomIconState NewState);
	
	/** Update visual appearance */
	UFUNCTION(BlueprintCallable, Category = "Room Icon")
	void UpdateVisuals();
	
	/** Play state change animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room Icon")
	void PlayStateChangeAnimation(ERoomIconState NewState);
	
	/** Play pulse animation (for current room) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room Icon")
	void PlayPulseAnimation();

protected:
	virtual void NativeConstruct() override;
};

/**
 * Connector line between room icons
 */
UCLASS()
class ATLAS_API URoomConnectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Connector line image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* ConnectorLine;
	
	/** Progress fill image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* ProgressFill;
	
	/** Set connection state */
	UFUNCTION(BlueprintCallable, Category = "Connector")
	void SetConnectionState(bool bCompleted);
	
	/** Play completion animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Connector")
	void PlayCompletionAnimation();
};

/**
 * Main run progress widget showing room progression and vital stats
 */
UCLASS()
class ATLAS_API URunProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS - ROOM PROGRESS
	// ========================================
	
	/** Container for room icons and connectors */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* RoomProgressContainer;
	
	/** Current level text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* CurrentLevelText;
	
	/** Current room name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* CurrentRoomName;
	
	/** Current enemy name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* CurrentEnemyName;
	
	/** Hazard warning container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* HazardWarningContainer;
	
	/** Hazard icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* HazardIcon;
	
	/** Hazard text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* HazardText;
	
	// ========================================
	// UI COMPONENTS - HEALTH
	// ========================================
	
	/** Player health bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* HealthBar;
	
	/** Health text (current/max) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* HealthText;
	
	/** Health icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* HealthIcon;
	
	// ========================================
	// UI COMPONENTS - STATION INTEGRITY
	// ========================================
	
	/** Station integrity bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* IntegrityBar;
	
	/** Integrity text (current/max) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* IntegrityText;
	
	/** Integrity icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* IntegrityIcon;
	
	/** Integrity warning (flashes when low) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* IntegrityWarning;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Widget class for room icons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TSubclassOf<URoomIconWidget> RoomIconClass;
	
	/** Widget class for connectors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TSubclassOf<URoomConnectorWidget> ConnectorClass;
	
	/** Low health threshold for warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float LowHealthThreshold = 0.25f;
	
	/** Low integrity threshold for warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float LowIntegrityThreshold = 0.25f;
	
	// ========================================
	// RUNTIME DATA
	// ========================================
	
	/** Reference to run manager */
	UPROPERTY(BlueprintReadOnly)
	URunManagerComponent* RunManager;
	
	/** Reference to health component */
	UPROPERTY(BlueprintReadOnly)
	UHealthComponent* HealthComponent;
	
	/** Reference to station integrity */
	UPROPERTY(BlueprintReadOnly)
	UStationIntegrityComponent* IntegrityComponent;
	
	/** Room icon widgets */
	UPROPERTY(BlueprintReadOnly)
	TArray<URoomIconWidget*> RoomIcons;
	
	/** Connector widgets */
	UPROPERTY(BlueprintReadOnly)
	TArray<URoomConnectorWidget*> Connectors;
	
	/** Current room index (0-4) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRoomIndex = 0;
	
	/** Current health value */
	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth = 100.0f;
	
	/** Max health value */
	UPROPERTY(BlueprintReadOnly)
	float MaxHealth = 100.0f;
	
	/** Current integrity value */
	UPROPERTY(BlueprintReadOnly)
	float CurrentIntegrity = 100.0f;
	
	/** Max integrity value */
	UPROPERTY(BlueprintReadOnly)
	float MaxIntegrity = 100.0f;
	
	// ========================================
	// DELEGATES
	// ========================================
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomChanged, int32, NewRoomIndex);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthCritical, float, HealthPercent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrityCritical, float, IntegrityPercent);
	
	/** Called when room changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoomChanged OnRoomChanged;
	
	/** Called when health reaches critical level */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthCritical OnHealthCritical;
	
	/** Called when integrity reaches critical level */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnIntegrityCritical OnIntegrityCritical;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize with game components */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void InitializeWithComponents(URunManagerComponent* InRunManager, 
		UHealthComponent* InHealth, UStationIntegrityComponent* InIntegrity);
	
	/** Create room progress UI */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void CreateRoomProgress();
	
	/** Update room progress display */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void UpdateRoomProgress(int32 NewRoomIndex);
	
	/** Update health display */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void UpdateHealthDisplay(float NewHealth, float NewMax);
	
	/** Update integrity display */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void UpdateIntegrityDisplay(float NewIntegrity, float NewMax);
	
	/** Update room info display */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void UpdateRoomInfo(URoomDataAsset* RoomData);
	
	/** Set room completed */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void SetRoomCompleted(int32 RoomIndex);
	
	/** Set room failed */
	UFUNCTION(BlueprintCallable, Category = "Run Progress")
	void SetRoomFailed(int32 RoomIndex);
	
	/** Flash health warning */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void FlashHealthWarning();
	
	/** Flash integrity warning */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void FlashIntegrityWarning();
	
	/** Play room transition animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayRoomTransitionAnimation();
	
	/** Play run complete animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayRunCompleteAnimation();
	
	/** Play run failed animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayRunFailedAnimation();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	/** Update hazard display */
	void UpdateHazardDisplay(ERoomHazard Hazard);
	
	/** Get hazard icon for type */
	UTexture2D* GetHazardIcon(ERoomHazard Hazard) const;
	
	/** Get hazard text for type */
	FText GetHazardText(ERoomHazard Hazard) const;
	
	/** Check for critical states */
	void CheckCriticalStates();
	
	/** Pulse current room icon */
	void PulseCurrentRoom();
	
	/** Timer for pulsing current room */
	FTimerHandle PulseTimerHandle;
};