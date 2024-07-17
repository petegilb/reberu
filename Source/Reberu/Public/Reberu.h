// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"

#define REBERU_LOG(LogLevel, Message) UE_LOG(LogReberu, LogLevel, TEXT(Message))
#define REBERU_LOG_ARGS(LogLevel, Message, ...) UE_LOG(LogReberu, LogLevel, TEXT(Message), __VA_ARGS__)

DECLARE_LOG_CATEGORY_EXTERN(LogReberu, Log, All);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(ReberuEmptyDoorTag)

class FReberuModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
