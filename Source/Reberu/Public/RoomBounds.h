// Copyright Peter Gilbert, All Rights Reserved

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* RoomBox;
	
	/**
	 * Door spawner vector that will be used for a gizmo.
	 * Referenced: https://forums.unrealengine.com/t/adding-custom-gizmos-to-actor-without-editor-extension/307597/6
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (MakeEditWidget = true))
	FVector DoorSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FReberuDoorList Doors;

	/** for use in the editor. represents the door we are currently editing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentlyEditingDoorId;

	/** Creates a new door that is locked to the extent of the box */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Door")
	void CreateNewDoor();

	/** Stop editing current door. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Door")
	void StopEditingDoor();

protected:
	virtual void BeginPlay() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Clamp the door gizmo to box bounds. */
	void LockDoorGizmo();

	/** Create a door or move a door if necessary. */
	void ManageDoor();
};
