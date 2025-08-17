#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MeleeAttack.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Jab UMETA(DisplayName = "Jab"),
	Heavy UMETA(DisplayName = "Heavy"),
	Combo UMETA(DisplayName = "Combo"),
	Random UMETA(DisplayName = "Random")
};

UCLASS()
class ATLAS_API UBTTask_MeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	EAttackType AttackType = EAttackType::Random;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	int32 ComboCount = 3;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float ComboDelay = 0.3f;

private:
	bool IsInAttackRange(class AAIController* AIController, class AActor* Target) const;
	void PerformAttack(class AEnemyCharacter* Enemy, EAttackType Type);
	void PerformCombo(class AEnemyCharacter* Enemy);
};