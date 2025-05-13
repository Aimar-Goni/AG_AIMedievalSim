// Copyright Epic Games, Inc. All Rights Reserved.

#include "AG_DynamicTimeAndSky.h"

#define LOCTEXT_NAMESPACE "FAG_DynamicTimeAndSkyModule"

void FAG_DynamicTimeAndSkyModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FAG_DynamicTimeAndSkyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAG_DynamicTimeAndSkyModule, AG_DynamicTimeAndSky)