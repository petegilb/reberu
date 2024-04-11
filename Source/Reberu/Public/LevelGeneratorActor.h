// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
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

	// FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform){
	// 	RoomData = InRoomData;
	// 	SpawnedTransform = InTransform;
	// }
	//
	// FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, FGuid InId){
	// 	RoomData = InRoomData;
	// 	SpawnedTransform = InTransform;
	// 	SpawnedRoomId = InId;
	// }
	//
	// UPROPERTY()
	// FGuid SpawnedRoomId;

	FReberuMove();
	// FReberuMove(UReberuRoomData* InRoomData, const FTransform& InTransform, )

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reberu")
	FRandomStream ReberuRandomStream;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reberu")
	TSubclassOf<ARoomBounds> RoomBoundsClass;

	UPROPERTY()
	bool bIsGenerating = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Reberu")
	TArray<ULevelStreamingDynamic*> LevelInstances;
	
	TDoubleLinkedList<FReberuMove> MovesList;

	// UPROPERTY()
	// TMap<FReberuMove*, FReberuRoom*> ;

	/** Does some prechecks before generation starts to see if we can even start generation. */
	bool CanStartGeneration() const;

	/** Do logic to place next room and retry accordingly. */
	void PlaceNextRoom();

	/** Chooses the next door to use during level generation. */
	TOptional<FReberuDoor> ChooseRoomDoor(const UReberuRoomData* InRoom) const;

	/** Chooses the next room during level generation. */
	UReberuRoomData* ChooseNextRoom() const;

	/** Retrieves a random door from the provided room. */
	TOptional<FReberuDoor> GetRandomDoor(const UReberuRoomData* InRoom) const;

	/** Retrieves a random room from reberu data */
	UReberuRoomData* GetRandomRoom() const;

	/** Spawn a room into the world by loading a level instance at the designated loc/rot. */
	ULevelStreamingDynamic* SpawnRoom(const UReberuRoomData* InRoom, const FTransform& SpawnTransform);

	/** Despawn a room from the world by unloading its instance */
	void DespawnRoom(ULevelStreamingDynamic* SpawnedRoom);

	/** Calculates the transform that the next room should spawn at by using their local transforms and the transforms of the doors */
	FTransform CalculateTransformFromDoor(ARoomBounds* CurrentRoomBounds, FReberuDoor CurrentRoomChosenDoor, UReberuRoomData* NextRoom, FReberuDoor NextRoomChosenDoor);

	/** Spawn in a room bounds instance using the specified size from the room data. */
	ARoomBounds* SpawnRoomBounds(const UReberuRoomData* InRoom, const FTransform& AtTransform);
};
