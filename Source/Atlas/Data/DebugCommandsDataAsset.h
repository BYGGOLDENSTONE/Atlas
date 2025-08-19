#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DebugCommandsDataAsset.generated.h"

UENUM(BlueprintType)
enum class EDebugCommandType : uint8
{
    Toggle,
    Action,
    Value,
    Info
};

USTRUCT(BlueprintType)
struct FDebugCommand
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString CommandName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString ConsoleCommand;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDebugCommandType CommandType = EDebugCommandType::Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Category = "General";

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bEnabledInShipping = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString DefaultValue = "";

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FString> ValidArguments;
};

UCLASS(BlueprintType)
class ATLAS_API UDebugCommandsDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Commands")
    TArray<FDebugCommand> DebugCommands;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    bool bAutoRegisterCommands = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FString CommandPrefix = "Atlas";

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void RegisterAllCommands();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnregisterAllCommands();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    TArray<FDebugCommand> GetCommandsByCategory(const FString& Category) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FDebugCommand GetCommandByName(const FString& CommandName) const;
};