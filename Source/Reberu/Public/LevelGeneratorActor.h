// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Data/ReberuData.h"
#include "Data/ReberuRoomData.h"
#include "GameFramework/Actor.h"
#include "LatentActions.h"
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
	
	FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, ARoomBounds* InTargetRoomBounds, bool InCanRevertMove){
		RoomData = InRoomData;
		SpawnedTransform = InTransform;
		TargetRoomBounds = InTargetRoomBounds;
		CanRevertMove = InCanRevertMove;
	}

	FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, ARoomBounds* InTargetRoomBounds, bool InCanRevertMove, ARoomBounds* InSourceRoomBounds,
		FString InSourceRoomDoor, FString InTargetRoomDoor)
	{
		RoomData = InRoomData;
		SpawnedTransform = InTransform;
		TargetRoomBounds = InTargetRoomBounds;
		CanRevertMove = InCanRevertMove;
		SourceRoomBounds = InSourceRoomBounds;
		SourceRoomDoor = InSourceRoomDoor;
		TargetRoomDoor = InTargetRoomDoor;
	}

	/** Reference to the room data associated with this move. */
	UReberuRoomData* RoomData {nullptr};

	/** The world transform that the room should be spawned at. */
	FTransform SpawnedTransform {FTransform()};
	
	ARoomBounds* SourceRoomBounds {nullptr};

	FString SourceRoomDoor {FString()};

	ARoomBounds* TargetRoomBounds {nullptr};

	FString TargetRoomDoor {FString()};

	ULevelStreamingDynamic* SpawnedLevel {nullptr};

	/** Specifies whether this move can be reverted during generation. */
	bool CanRevertMove = true; 
};

/**
 * Level Generator used for Reberu Level Generation! 
 */
UCLASS()
class REBERU_API ALevelGeneratorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelGeneratorActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Reberu")
	virtual void StartGeneration();

	UFUNCTION(BlueprintImplementableEvent)
	void K2_StartGeneration();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Reberu")
	virtual void ClearGeneration();

	/** Spawn in a room bounds instance using the specified size from the room data. */
	ARoomBounds* SpawnRoomBounds(const UReberuRoomData* InRoom, const FTransform& AtTransform);

	/** Do logic to place next room and retry accordingly. */
	bool PlaceNextRoom(UReberuData* ReberuData, FReberuMove& NewMove, ARoomBounds* SourceRoomBounds, TSet<FString>& AttemptedTargetRoomDoors, TSet<UReberuRoomData*>& AttemptedTargetRooms, TSet<FString>&
	                   AttemptedSourceRoomDoors);

	/** Choose the next source room if possible (or keep the current one). Only returns false on failure. Uses the inputted selection type. */
	virtual bool ChooseSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomSelection SelectionType, bool bFromError=false);

	/** Backtrack by moving back on the moveslist. Method type can be specified and overridden. We assume we have at least 2 rooms so we can actually backtrack. */
	virtual bool BacktrackSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomBacktrack BacktrackMethod, TSet<UReberuRoomData*>& AttemptedTargetRooms);
	
	/** Spawn a room into the world by loading a level instance at the designated loc/rot. */
	ULevelStreamingDynamic* SpawnRoom(const UReberuRoomData* InRoom, const FTransform& SpawnTransform, FString LevelName);

	/** Despawn a room from the world by unloading its instance */
	void DespawnRoom(ULevelStreamingDynamic* SpawnedRoom);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	UBillboardComponent* SpriteComponent;

	//TODO actually use this
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	UTexture2D* SpriteTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	bool bStartOnBeginPlay = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reberu")
	FRandomStream ReberuRandomStream;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	TSubclassOf<ARoomBounds> RoomBoundsClass;

	UPROPERTY()
	bool bIsGenerating = false;
	
	TDoubleLinkedList<FReberuMove> MovesList;

	/** Does some prechecks before generation starts to see if we can even start generation. */
	virtual bool CanStartGeneration() const;

	/** Calculates the transform that the next room should spawn at by using their local transforms and the transforms of the doors */
	FTransform CalculateTransformFromDoor(ARoomBounds* CurrentRoomBounds, FReberuDoor CurrentRoomChosenDoor, UReberuRoomData* NextRoom, FReberuDoor NextRoomChosenDoor);

public:
	TDoubleLinkedList<FReberuMove>& GetMovesListRef(){return MovesList;}

	FRandomStream& GetReberuRandomStream(){return ReberuRandomStream;}

	bool IsGenerating() const{return bIsGenerating;}
	void SetIsGenerating(const bool InBool){bIsGenerating = InBool;}
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
