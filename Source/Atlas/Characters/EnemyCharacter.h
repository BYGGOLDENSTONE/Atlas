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

public:
	virtual void Tick(float DeltaTime) override;
};