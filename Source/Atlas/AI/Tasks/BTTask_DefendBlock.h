#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DefendBlock.generated.h"

UENUM(BlueprintType)
enum class EDefenseType : uint8
{
	Block UMETA(DisplayName = "Block"),
	Parry UMETA(DisplayName = "Parry"),
	Dodge UMETA(DisplayName = "Dodge"),
	Auto UMETA(DisplayName = "Auto-Decide")
};

UCLASS()
class ATLAS_API UBTTask_DefendBlock : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DefendBlock();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Defense")
	EDefenseType DefenseType = EDefenseType::Auto;

	UPROPERTY(EditAnywhere, Category = "Defense")
	float BlockDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Defense")
	float ParryWindow = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Defense")
	float ParrySuccessChance = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Defense")
	float DodgeDistance = 300.0f;

private:
	float TimeElapsed;
	bool bIsBlocking;
	bool bParryAttempted;

	void StartBlock(class AEnemyCharacter* Enemy);
	void StopBlock(class AEnemyCharacter* Enemy);
	bool AttemptParry(class AEnemyCharacter* Enemy);
	void PerformDodge(class AEnemyCharacter* Enemy, class AActor* Threat);
	EDefenseType DecideDefenseType(class UBlackboardComponent* BlackboardComp);
};