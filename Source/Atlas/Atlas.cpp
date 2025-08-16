// Copyright Epic Games, Inc. All Rights Reserved.

#include "Atlas.h"
#include "Modules/ModuleManager.h"
#include "Core/AtlasGameplayTags.h"

class FAtlasModule : public FDefaultGameModuleImpl
{
    virtual void StartupModule() override
    {
        FAtlasGameplayTags::InitializeNativeTags();
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE( FAtlasModule, Atlas, "Atlas" );