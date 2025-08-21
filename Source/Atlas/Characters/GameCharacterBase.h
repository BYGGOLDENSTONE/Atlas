#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interfaces/ICombatInterface.h"
#include "../Interfaces/IHealthInterface.h"
#include "../Interfaces/IActionInterface.h"
#include "GameCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UActionManagerComponent;

UCLASS(Abstract)
class ATLAS_API AGameCharacterBase : public ACharacter, public ICombatInterface, public IHealthInterface, public IActionInterface
{
	GENERATED_BODY()

public:
	AGameCharacterBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actions, meta = (AllowPrivateAccess = "true"))
	UActionManagerComponent* ActionManagerComponent;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE class UActionManagerComponent* GetActionManagerComponent() const { return ActionManagerComponent; }
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	
	// ICombatInterface implementation
	virtual bool IsInCombat_Implementation() const override;
	virtual bool IsAttacking_Implementation() const override;
	virtual bool IsBlocking_Implementation() const override;
	virtual bool IsVulnerable_Implementation() const override;
	virtual bool HasIFrames_Implementation() const override;
	virtual void AddCombatStateTag_Implementation(const FGameplayTag& Tag) override;
	virtual void RemoveCombatStateTag_Implementation(const FGameplayTag& Tag) override;
	virtual bool HasCombatStateTag_Implementation(const FGameplayTag& Tag) const override;
	virtual bool StartAttack_Implementation(const FGameplayTag& AttackTag) override;
	virtual void EndAttack_Implementation() override;
	virtual bool StartBlock_Implementation() override;
	virtual void EndBlock_Implementation() override;
	
	// IHealthInterface implementation
	virtual float GetCurrentHealth_Implementation() const override;
	virtual float GetMaxHealth_Implementation() const override;
	virtual float GetHealthPercent_Implementation() const override;
	virtual bool IsAlive_Implementation() const override;
	virtual bool IsDead_Implementation() const override;
	virtual void ApplyDamage_Implementation(float DamageAmount, AActor* DamageInstigator) override;
	virtual void ApplyHealing_Implementation(float HealAmount, AActor* Healer) override;
	
	// IActionInterface implementation
	virtual bool CanPerformAction_Implementation(const FGameplayTag& ActionTag) const override;
	virtual bool TryPerformAction_Implementation(const FGameplayTag& ActionTag) override;
	virtual void InterruptCurrentAction_Implementation() override;
	virtual bool IsPerformingAction_Implementation() const override;
	virtual UBaseAction* GetCurrentAction_Implementation() const override;
	virtual bool AssignActionToSlot_Implementation(FName SlotName, const FGameplayTag& ActionTag) override;
	virtual void ClearActionSlot_Implementation(FName SlotName) override;
	virtual UBaseAction* GetActionInSlot_Implementation(FName SlotName) const override;
};