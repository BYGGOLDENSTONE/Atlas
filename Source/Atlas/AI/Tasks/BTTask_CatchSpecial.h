#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CatchSpecial.generated.h"

UCLASS()
class ATLAS_API UBTTask_CatchSpecial : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CatchSpecial();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Catch Special")
	float DashSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Catch Special")
	float MaxDashDistance = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Catch Special")
	float CatchRadius = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Catch Special")
	float StunDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Catch Special")
	UAnimMontage* DashMontage;

private:
	FVector StartLocation;
	FVector TargetLocation;
	float DashTimeElapsed;
	bool bDashComplete;
	class AEnemyCharacter* CachedEnemy;
	class AActor* CachedTarget;

	void InitiateDash(class AEnemyCharacter* Enemy, class AActor* Target);
	void CheckCatchCollision();
	void OnCatchSuccess(class AActor* CaughtTarget);
};