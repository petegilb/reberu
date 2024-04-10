// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ReberuRule.generated.h"

class UReberuRoomData;
/**
 * Abstract Base class for rules to be used in transitions.
 * Should be used to contain logic on whether or not a room can be placed.
 * Override 
 */
UCLASS(Abstract, Blueprintable)
class REBERU_API UReberuRule : public UObject{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	bool ShouldPlaceRoom(UReberuRoomData* OwningRoom, UReberuRoomData* ConnectingRoom);
};
