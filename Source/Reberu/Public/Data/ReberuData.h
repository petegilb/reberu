// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ReberuData.generated.h"

class UReberuRoomData;

/**
 * Data asset containing rooms to be generated using Reberu.
 */
UCLASS()
class REBERU_API UReberuData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UReberuRoomData* StartingRoom;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UReberuRoomData*> ReberuRooms;

	/** The target amount of rooms that should exist in the generated level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 TargetRoomAmount = 10;
};
