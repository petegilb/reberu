// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Settings/ReberuSettings.h"
#include "ReberuRoomData.generated.h"

class UReberuRule;
/**
 * Represents a door in a room for Reberu generation.
 */
USTRUCT(BlueprintType, Blueprintable)
struct FReberuDoor{
	GENERATED_BODY()

	FReberuDoor(){
		DoorId = "";
		GenerateNewDoorId();
		const UReberuSettings* ReberuSettings = GetDefault<UReberuSettings>();
		BoxExtent = ReberuSettings->DefaultDoorExtent;
		DoorTag = FGameplayTag::EmptyTag;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString DoorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform DoorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxExtent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag DoorTag;

	/** bool specifying if this door can only connect to doors of the same type. empty tags won't be limited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyConnectSameDoor = true;
	
	void GenerateNewDoorId(){
		DoorId = FGuid::NewGuid().ToString();
	}
};

/**
 * Represents a Room for Reberu level generation. This struct should be created using the editor tools.
 * The final struct will live in a UReberuRoomData data asset but should be generated with a BP_RoomBounds.
 */
USTRUCT(BlueprintType, Blueprintable)
struct FReberuRoom{
	GENERATED_BODY()

	/** The Level to be associated with this room. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level = nullptr;

	/** The location in world space of the box in the context of the current room. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTransform BoxActorTransform;

	/** The extent of the box collision necessary for checking for room overlaps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector BoxExtent;

	/** The doors associated with this room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FReberuDoor> ReberuDoors;

	/** Set containing the used doors (key is their id). Used in generation. */
	UPROPERTY()
	TSet<FString> UsedDoors;

	/** Tags associated with this room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RoomTags;

	/** bool specifying if room type should be allowed to connect to its same type of room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowSameRoomConnect = false;

	/** Depth of the node from the head */
	UPROPERTY(BlueprintReadOnly)
	int32 Depth = 0; 

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reberu|Room", meta = (ToolTip = "The doors associated with this room."))
	FReberuRoom Room;
	
};
