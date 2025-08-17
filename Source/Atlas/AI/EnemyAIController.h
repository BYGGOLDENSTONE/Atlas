#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboardData;

UCLASS()
class ATLAS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBlackboardData* BlackboardData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	float DefendRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	float SoulAttackCooldown = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	float CatchSpecialRange = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	float ParryReactionTime = 0.2f;

private:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetupPerceptionSystem();
	void InitializeBlackboardData();

	FTimerHandle SoulAttackCooldownTimer;
	FTimerHandle LostTargetTimer;
	bool bCanUseSoulAttack = true;

public:
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool CanUseSoulAttack() const { return bCanUseSoulAttack; }

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void StartSoulAttackCooldown();

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	float GetDefendRange() const { return DefendRange; }

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	float GetCatchSpecialRange() const { return CatchSpecialRange; }

private:
	void ResetSoulAttackCooldown();
};