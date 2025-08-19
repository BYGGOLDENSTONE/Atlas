#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "DashComponent.generated.h"

USTRUCT(BlueprintType)
struct FDashSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float DashDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float DashDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float DashCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bEnableInvincibility = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float InvincibilityDuration = 0.5f;
};

UCLASS(BlueprintType)
class ATLAS_API UDashDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	FDashSettings DashSettings;
};

UENUM(BlueprintType)
enum class EDashState : uint8
{
	Ready,
	Dashing,
	Cooldown
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDashComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Dash")
	void TryDash(const FVector2D& InputDirection);

	UFUNCTION(BlueprintCallable, Category = "Dash")
	bool CanDash() const;

	UFUNCTION(BlueprintPure, Category = "Dash")
	EDashState GetDashState() const { return CurrentDashState; }

	UFUNCTION(BlueprintPure, Category = "Dash")
	float GetCooldownRemaining() const { return CooldownTimer; }

	UFUNCTION(BlueprintPure, Category = "Dash")
	bool IsInvincible() const { return bIsInvincible; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	UDashDataAsset* DashDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (EditCondition = "!DashDataAsset"))
	FDashSettings DefaultDashSettings;

private:
	void StartDash(const FVector& Direction);
	void UpdateDash(float DeltaTime);
	void EndDash();
	bool CheckStateRestrictions() const;
	FVector CalculateDashDirection(const FVector2D& InputDirection) const;
	bool CheckForCollision(const FVector& TestLocation) const;
	void SetInvincibility(bool bEnable);

	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCombatComponent* CombatComponent;

	UPROPERTY()
	class UHealthComponent* HealthComponent;

	EDashState CurrentDashState;
	FVector DashStartLocation;
	FVector DashTargetLocation;
	FVector DashDirection;
	float DashTimer;
	float CooldownTimer;
	float InvincibilityTimer;
	bool bIsInvincible;

	FDashSettings GetDashSettings() const;
};