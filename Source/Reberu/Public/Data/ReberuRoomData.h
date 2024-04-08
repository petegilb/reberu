// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ReberuRoomData.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FReberuDoor{
	GENERATED_BODY()

	FReberuDoor(){
		GenerateNewDoorId();
		Weight = 0.f;
		BoxExtent = FVector(20.f, 50.f, 100.f);
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString DoorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform DoorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxExtent;

	/** The weight of how this should be selected (should be out of 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;
	
	void GenerateNewDoorId(){
		DoorId = FGuid::NewGuid().ToString();
	}
};

USTRUCT(BlueprintType, Blueprintable)
struct FReberuDoorList{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FReberuDoor> ReberuDoors;

	FReberuDoor* GetDoorById(FString InId){
		FReberuDoor* CurrentDoor = ReberuDoors.FindByPredicate([InId](const FReberuDoor& InItem)
		{
			return InItem.DoorId == InId;
		});
		return CurrentDoor;
	}

	int32 GetDoorIdxById(FString InId) const{
		const int32 CurrentDoorIdx = ReberuDoors.IndexOfByPredicate([InId](const FReberuDoor& InItem)
		{
			return InItem.DoorId == InId;
		});
		return CurrentDoorIdx;
	}
	
};

/**
 * This data asset represents a "room" for our map generation.
 */
UCLASS()
class REBERU_API UReberuRoomData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The FName to be associated with this room."))
	FName RoomName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The Level to be associated with this room."))
	TSoftObjectPtr<UWorld> Room = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The extent of the box collision necessary for checking for room overlaps."))
	FVector BoxExtent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The location in world space of the box in the context of the current room."))
	FVector BoxLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The doors associated with this room."))
	FReberuDoorList RoomDoors;
	

protected:

	
	
};
