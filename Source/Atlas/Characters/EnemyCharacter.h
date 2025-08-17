#pragma once

#include "CoreMinimal.h"
#include "GameCharacterBase.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class ATLAS_API AEnemyCharacter : public AGameCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	class UAnimMontage* SoulAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	class UAnimMontage* DashMontage;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	class UAnimMontage* GetSoulAttackMontage() const { return SoulAttackMontage; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	class UAnimMontage* GetDashMontage() const { return DashMontage; }
};