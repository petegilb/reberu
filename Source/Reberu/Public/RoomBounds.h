﻿// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Data/ReberuRoomData.h"
#include "GameFramework/Actor.h"
#include "RoomBounds.generated.h"

class UBoxComponent;

UCLASS()
class REBERU_API ARoomBounds : public AActor{
	GENERATED_BODY()

public:
	ARoomBounds();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DoorEditor")
	UBoxComponent* RoomBox;
	
	/**
	 * Door spawner vector that will be used for a gizmo.
	 * Referenced: https://forums.unrealengine.com/t/adding-custom-gizmos-to-actor-without-editor-extension/307597/6
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (MakeEditWidget = true), Category="DoorEditor")
	FTransform DoorSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DoorEditor", meta=(ShowOnlyInnerProperties))
	FReberuRoom Room;

	/** for use in the editor. represents the door we are currently editing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DoorEditor")
	FString CurrentlyEditingDoorId;

	/** Creates a new door that is locked to the extent of the box */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="DoorEditor", meta=(DisplayPriority=0))
	void CreateNewDoor();
	
	/** Only exists to act as in input for EditDoorAtIdx and DelDoorAtIdx. Don't use for any other purpose. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="DoorEditor", meta=(DisplayPriority=0))
	int32 EditDoorIdx = 0;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="DoorEditor", meta=(DisplayPriority=1))
	bool LockGizmoToBounds = true;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="DoorEditor", meta=(DisplayPriority=1))
	void EditDoorAtIdx();

	/** Stop editing current door and reset gizmo position. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="DoorEditor", meta=(DisplayPriority=2))
	void StopEditingDoor();
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="DoorEditor", meta=(DisplayPriority=3))
	void DelDoorAtIdx();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="DoorEditor", meta=(DisplayPriority=4))
	void RegenerateDoorIds();

protected:
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	/** Clamp the door gizmo to box bounds. */
	void LockDoorGizmo();

	/** Create a door or move a door if necessary. */
	void ManageDoor();
};
