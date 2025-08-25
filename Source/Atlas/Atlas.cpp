// Copyright Epic Games, Inc. All Rights Reserved.

#include "Atlas.h"
#include "Modules/ModuleManager.h"
#include "Core/AtlasGameplayTags.h"
#include "Debug/AtlasConsoleCommands.h"

class FAtlasModule : public FDefaultGameModuleImpl
{
    virtual void StartupModule() override
    {
        FAtlasGameplayTags::InitializeNativeTags();
        FAtlasConsoleCommands::RegisterCommands();
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE( FAtlasModule, Atlas, "Atlas" );