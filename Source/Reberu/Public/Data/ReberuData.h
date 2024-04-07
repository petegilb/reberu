// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ReberuData.generated.h"

class UReberuRoomData;

/**
 * 
 */
UCLASS()
class REBERU_API UReberuData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UReberuRoomData*> ReberuRooms;

protected:
	
};
