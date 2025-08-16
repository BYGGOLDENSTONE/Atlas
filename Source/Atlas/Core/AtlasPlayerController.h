#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AtlasPlayerController.generated.h"

UCLASS()
class ATLAS_API AAtlasPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAtlasPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};