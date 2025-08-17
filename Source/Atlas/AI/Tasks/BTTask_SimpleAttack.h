#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SimpleAttack.generated.h"

UCLASS()
class ATLAS_API UBTTask_SimpleAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SimpleAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float Damage = 5.0f;
};