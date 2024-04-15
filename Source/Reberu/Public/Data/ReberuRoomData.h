// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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
		Weight = 0.f;
		BoxExtent = FVector(5.f, 50.f, 100.f);
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

/**
 * Represents a connection from one room to another.
 * If a room has no transitions, a random room in the data asset will be picked.
 */
USTRUCT(BlueprintType, Blueprintable)
struct FReberuTransition{
	GENERATED_BODY()

	/** The room that the owning room should transition to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UReberuRoomData* ConnectedRoom;

	/** Weight represents the likelihood that this transition will be chosen. Should be between 0-1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Weight;

	/** Rules that specify whether or not this transition can be used. No rules == good to go. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TSubclassOf<UReberuRule>> Rules;
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

	/** The rooms that we can transition to from this room. None means a random one will be chosen. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FReberuTransition> Transitions;

	/** Set containing the used doors (key is their id). Used in generation. */
	UPROPERTY()
	TSet<FString> UsedDoors;

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
