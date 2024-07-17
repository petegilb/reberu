// Copyright Peter Gilbert, All Rights Reserved

#include "Reberu.h"

#define LOCTEXT_NAMESPACE "FReberuModule"

DEFINE_LOG_CATEGORY(LogReberu);
UE_DEFINE_GAMEPLAY_TAG(ReberuEmptyDoorTag, "Reberu.Door.Empty")

void FReberuModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FReberuModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FReberuModule, Reberu)