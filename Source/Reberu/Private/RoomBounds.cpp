// Copyright Peter Gilbert, All Rights Reserved


#include "RoomBounds.h"

#include "Components/BoxComponent.h"


ARoomBounds::ARoomBounds(){
	PrimaryActorTick.bCanEverTick = false;

	RoomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBox"));
	SetRootComponent(RoomBox);
	RoomBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RoomBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RoomBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
}

void ARoomBounds::CreateNewDoor(){
	const FReberuDoor NewDoor = FReberuDoor();
	Doors.ReberuDoors.Add(NewDoor);
	CurrentlyEditingDoorId = NewDoor.DoorId;
}

void ARoomBounds::EditDoorAtIdx(){
	const FReberuDoor& NewDoor = Doors.ReberuDoors[EditDoorIdx];
	DoorSpawn = NewDoor.DoorTransform;
	CurrentlyEditingDoorId = NewDoor.DoorId;
}

void ARoomBounds::StopEditingDoor(){
	CurrentlyEditingDoorId = "";
	DoorSpawn = FTransform();
}

void ARoomBounds::BeginPlay(){
	Super::BeginPlay();
	
}

void ARoomBounds::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent){

	//Get the name of the property that was changed  
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;  

	//Check if it's the property we want  
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(ARoomBounds, DoorSpawn)))  
	{  
		LockDoorGizmo();
		ManageDoor();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ARoomBounds::LockDoorGizmo(){
	
	
}

void ARoomBounds::ManageDoor(){
	FReberuDoor* CurrentDoor = Doors.GetDoorById(CurrentlyEditingDoorId);

	if(!CurrentDoor) return;
	
	CurrentDoor->DoorTransform = DoorSpawn;
}
