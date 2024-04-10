// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ReberuSettings.generated.h"

/**
 * Settings for Reberu.
 */
UCLASS(config = EditorUserSettings, defaultconfig)
class REBERUEDITOR_API UReberuSettings : public UObject{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category="Reberu")
	FString ReberuPath = "/Game/Reberu";
};
