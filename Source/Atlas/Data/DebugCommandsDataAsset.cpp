#include "DebugCommandsDataAsset.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

void UDebugCommandsDataAsset::RegisterAllCommands()
{
#if !UE_BUILD_SHIPPING
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    
    for (const FDebugCommand& Command : DebugCommands)
    {
        FString FullCommand = FString::Printf(TEXT("%s.%s"), *CommandPrefix, *Command.CommandName);
        
        if (!ConsoleManager.FindConsoleVariable(*FullCommand))
        {
            switch (Command.CommandType)
            {
                case EDebugCommandType::Toggle:
                {
                    bool DefaultBool = Command.DefaultValue.ToBool();
                    ConsoleManager.RegisterConsoleVariable(
                        *FullCommand,
                        DefaultBool,
                        *Command.Description,
                        ECVF_Default
                    );
                    break;
                }
                case EDebugCommandType::Value:
                {
                    float DefaultFloat = FCString::Atof(*Command.DefaultValue);
                    ConsoleManager.RegisterConsoleVariable(
                        *FullCommand,
                        DefaultFloat,
                        *Command.Description,
                        ECVF_Default
                    );
                    break;
                }
                case EDebugCommandType::Action:
                case EDebugCommandType::Info:
                {
                    ConsoleManager.RegisterConsoleCommand(
                        *FullCommand,
                        *Command.Description,
                        FConsoleCommandDelegate::CreateLambda([Command]()
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Debug Command Executed: %s"), *Command.CommandName);
                        }),
                        ECVF_Default
                    );
                    break;
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Registered debug command: %s"), *FullCommand);
        }
    }
#endif
}

void UDebugCommandsDataAsset::UnregisterAllCommands()
{
#if !UE_BUILD_SHIPPING
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    
    for (const FDebugCommand& Command : DebugCommands)
    {
        FString FullCommand = FString::Printf(TEXT("%s.%s"), *CommandPrefix, *Command.CommandName);
        ConsoleManager.UnregisterConsoleObject(*FullCommand);
    }
#endif
}

TArray<FDebugCommand> UDebugCommandsDataAsset::GetCommandsByCategory(const FString& Category) const
{
    TArray<FDebugCommand> FilteredCommands;
    
    for (const FDebugCommand& Command : DebugCommands)
    {
        if (Command.Category.Equals(Category, ESearchCase::IgnoreCase))
        {
            FilteredCommands.Add(Command);
        }
    }
    
    return FilteredCommands;
}

FDebugCommand UDebugCommandsDataAsset::GetCommandByName(const FString& CommandName) const
{
    for (const FDebugCommand& Command : DebugCommands)
    {
        if (Command.CommandName.Equals(CommandName, ESearchCase::IgnoreCase))
        {
            return Command;
        }
    }
    
    return FDebugCommand();
}