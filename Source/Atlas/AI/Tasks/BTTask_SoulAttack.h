#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SoulAttack.generated.h"

UCLASS()
class ATLAS_API UBTTask_SoulAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SoulAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Soul Attack")
	float ChargeTime = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Soul Attack")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Soul Attack")
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Soul Attack")
	UAnimMontage* SoulAttackMontage;

private:
	void ExecuteSoulAttack(class AEnemyCharacter* Enemy, class AActor* Target);
};