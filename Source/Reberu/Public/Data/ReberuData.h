// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ReberuData.generated.h"

class UReberuRoomData;

UENUM(BlueprintType)
enum class ERoomSelection : uint8 {
	Breadth,
	Depth,
	Random,
	Custom1,
	Custom2,
	Custom3,
	Custom4,
};

UENUM(BlueprintType)
enum class ERoomBacktrack : uint8 {
	FromTail,
	UntilCurrent,
	None,
	Custom1,
	Custom2,
	Custom3,
	Custom4,
};

USTRUCT(Blueprintable, BlueprintType)
struct FReberuDoorInfo{
	GENERATED_BODY()

	/** */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> DoorActor;

	/** */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> BlockedDoorActor;
};

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

	/** The minimum amount of rooms required to finish generation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MinRoomAmount = 5;

	/** The maximum amount of times we can backtrack (in a row) before failing generation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxBacktrackTries = 5;

	/** The method to select the source room during generation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	ERoomSelection RoomSelectionMethod = ERoomSelection::Breadth;

	/** The method to backtrack upon failure. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	ERoomBacktrack BacktrackMethod = ERoomBacktrack::FromTail;

	/** Map to specify door related info mapped to a tag. Add to Reberu.Door.Empty for non tagged doors */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FGameplayTag, FReberuDoorInfo> DoorMap;
};
