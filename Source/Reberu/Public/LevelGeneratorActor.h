// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Data/ReberuData.h"
#include "Data/ReberuRoomData.h"
#include "GameFramework/Actor.h"
#include "LevelGeneratorActor.generated.h"

class ARoomBounds;
struct FReberuDoor;
class ULevelStreamingDynamic;
class UReberuRoomData;
class UReberuData;


/** 
 * Struct representing a move in Reberu Level generation.
 * To be used within our doubly linked list.
 */
struct FReberuMove{

	FReberuMove(){
		
	}

	FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform){
		RoomData = InRoomData;
		SpawnedTransform = InTransform;
	}
	
	FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, ARoomBounds* InToRoomBounds, bool InCanRevertMove){
		RoomData = InRoomData;
		SpawnedTransform = InTransform;
		ToRoomBounds = InToRoomBounds;
		CanRevertMove = InCanRevertMove;
	}

	FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, ARoomBounds* InToRoomBounds, bool InCanRevertMove, ARoomBounds* InFromRoomBounds,
		FString InFromRoomDoor, FString InToRoomDoor)
	{
		RoomData = InRoomData;
		SpawnedTransform = InTransform;
		ToRoomBounds = InToRoomBounds;
		CanRevertMove = InCanRevertMove;
		FromRoomBounds = InFromRoomBounds;
		FromRoomDoor = InFromRoomDoor;
		ToRoomDoor = InToRoomDoor;
	}

	/** Reference to the room data associated with this move. */
	UReberuRoomData* RoomData {nullptr};

	/** The world transform that the room should be spawned at. */
	FTransform SpawnedTransform {FTransform()};
	
	ARoomBounds* FromRoomBounds {nullptr};

	FString FromRoomDoor {FString()};

	ARoomBounds* ToRoomBounds {nullptr};

	FString ToRoomDoor {FString()};

	/** Specifies whether this move can be reverted during generation. */
	bool CanRevertMove = true; 
};

UCLASS()
class REBERU_API ALevelGeneratorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelGeneratorActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Reberu")
	void StartGeneration();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Reberu")
	void ClearGeneration();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	UBillboardComponent* SpriteComponent;

	//TODO actually use this
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	UTexture2D* SpriteTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	UReberuData* ReberuData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	bool bStartOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reberu")
	int32 ReberuSeed = -1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reberu")
	FRandomStream ReberuRandomStream;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	TSubclassOf<ARoomBounds> RoomBoundsClass;

	UPROPERTY()
	bool bIsGenerating = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Reberu")
	TArray<ULevelStreamingDynamic*> LevelInstances;
	
	TDoubleLinkedList<FReberuMove> MovesList;

	/** Does some prechecks before generation starts to see if we can even start generation. */
	bool CanStartGeneration() const;

	/** Do logic to place next room and retry accordingly. */
	bool PlaceNextRoom(FReberuMove& NewMove, ARoomBounds* FromRoomBounds, TSet<FString>& AttemptedNewRoomDoors, TSet<UReberuRoomData*>& AttemptedNewRooms, TSet<FString>& AttemptedOldRoomDoors);

	/** Spawn a room into the world by loading a level instance at the designated loc/rot. */
	ULevelStreamingDynamic* SpawnRoom(const UReberuRoomData* InRoom, const FTransform& SpawnTransform);

	/** Despawn a room from the world by unloading its instance */
	void DespawnRoom(ULevelStreamingDynamic* SpawnedRoom);

	/** Calculates the transform that the next room should spawn at by using their local transforms and the transforms of the doors */
	FTransform CalculateTransformFromDoor(ARoomBounds* CurrentRoomBounds, FReberuDoor CurrentRoomChosenDoor, UReberuRoomData* NextRoom, FReberuDoor NextRoomChosenDoor);

	/** Spawn in a room bounds instance using the specified size from the room data. */
	ARoomBounds* SpawnRoomBounds(const UReberuRoomData* InRoom, const FTransform& AtTransform);

	/** Choose the next source room if possible (or keep the current one). Only returns false on failure. Uses the inputted selection type. */
	virtual bool ChooseSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomSelection SelectionType, bool bFromError=false);

	/** Backtrack by moving back on the moveslist. Method type can be specified and overridden. We assume we have at least 2 rooms so we can actually backtrack. */
	virtual bool BacktrackSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomBacktrack BacktrackMethod, TSet<UReberuRoomData*>& AttemptedNewRooms);
};

/** Helper function to get a random object in an array using our random stream. */
template<typename T>
static T& GetRandomObjectInArray(TArray<T>& InArray, FRandomStream& InRandomStream){
	return InArray[InRandomStream.RandRange(0, InArray.Num() - 1)];
}

/** Helper function to get a random object in a set using our random stream. */
template<typename T>
static T& GetRandomObjectInSet(TSet<T>& InSet, FRandomStream& InRandomStream){
	return InSet[InRandomStream.RandRange(0, InSet.Num() - 1)];
}
