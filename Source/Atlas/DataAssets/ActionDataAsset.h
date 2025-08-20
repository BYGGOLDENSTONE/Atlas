#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActionDataAsset.generated.h"

class UBaseAction;

/**
 * Data asset containing configuration for player actions.
 * All action stats and properties are driven by these data assets.
 */
UCLASS(BlueprintType)
class ATLAS_API UActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Core Identity
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Identity")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Identity")
	TSubclassOf<UBaseAction> ActionClass;

	// Display Information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Display")
	FText ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Display", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Display")
	UTexture2D* Icon;

	// Core Stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Stats", meta = (ClampMin = 0.0))
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Stats", meta = (ClampMin = 0.0))
	float IntegrityCost = 0.0f;

	// Combat Stats (used by combat actions)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Combat", meta = (ClampMin = 0.0))
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Combat", meta = (ClampMin = 0.0))
	float KnockbackForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Combat", meta = (ClampMin = 0.0))
	float PoiseDamage = 0.0f;

	// Ability Stats (used by various abilities)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Ability", meta = (ClampMin = 0.0))
	float Range = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Ability", meta = (ClampMin = 0.0))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Ability", meta = (ClampMin = 0.0))
	float Radius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Ability", meta = (ClampMin = 0.0))
	float ChargeTime = 0.0f;

	// Movement Stats (used by movement actions)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Movement", meta = (ClampMin = 0.0))
	float MoveDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Movement", meta = (ClampMin = 0.0))
	float MoveSpeed = 0.0f;

	// Requirements and Restrictions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Requirements")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Requirements")
	FGameplayTagContainer BlockedDuringTags;

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Animation")
	class UAnimMontage* ActionMontage;

	// Visual Effects
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Effects")
	class UParticleSystem* ActivationEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Effects")
	class USoundBase* ActivationSound;

	// Additional customizable parameters (for flexibility)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Custom")
	TMap<FName, float> CustomFloatParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Custom")
	TMap<FName, FString> CustomStringParameters;
};