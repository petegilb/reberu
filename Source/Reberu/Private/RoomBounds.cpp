// Copyright Peter Gilbert, All Rights Reserved


#include "RoomBounds.h"

#include "Reberu.h"
#include "Components/BoxComponent.h"


ARoomBounds::ARoomBounds(){
	PrimaryActorTick.bCanEverTick = false;

	RoomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBox"));
	SetRootComponent(RoomBox);
	RoomBox->SetCollisionObjectType(ECC_WorldStatic);
	RoomBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RoomBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RoomBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
}

void ARoomBounds::CreateNewDoor(){
	const FReberuDoor NewDoor = FReberuDoor();
	Room.ReberuDoors.Add(NewDoor);
	CurrentlyEditingDoorId = NewDoor.DoorId;
}

void ARoomBounds::EditDoorAtIdx(){
	const FReberuDoor& NewDoor = Room.ReberuDoors[EditDoorIdx];
	DoorSpawn = NewDoor.DoorTransform;
	CurrentlyEditingDoorId = NewDoor.DoorId;
}

void ARoomBounds::StopEditingDoor(){
	ManageDoor();
	CurrentlyEditingDoorId = "";
	DoorSpawn = FTransform();
}

void ARoomBounds::DelDoorAtIdx(){
	if (!Room.ReberuDoors.IsValidIndex(EditDoorIdx)) return;

	Room.ReberuDoors.RemoveAt(EditDoorIdx);
}

void ARoomBounds::RegenerateDoorIds(){
	for(FReberuDoor& Door : Room.ReberuDoors){
		Door.GenerateNewDoorId();
	}
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
	const FBox Box = FBox( -RoomBox->GetScaledBoxExtent(), RoomBox->GetScaledBoxExtent() );
	FVector Point = DoorSpawn.GetLocation();

	/** If the gizmo is not inside the box. let's snap it back */
	bool IsOnXWall = false;
	bool IsOnYWall = false;

	if (Point.X == Box.Min.X || Point.X == Box.Max.X){
		IsOnXWall = true;
	}
	if (Point.Y == Box.Min.Y || Point.Y == Box.Max.Y){
		IsOnYWall = true;
	}

	if(!IsOnXWall && !IsOnYWall){
		Point.X = Box.Min.X;
		Point.Y = Box.Min.Y;
	}
	else if (IsOnXWall){
		const double MinDiff = abs(Point.X - Box.Min.X);
		const double MaxDiff = abs(Point.X - Box.Max.X);
		Point.X = MinDiff < MaxDiff ? Box.Min.X : Box.Max.X;

		Point.Y = FMath::Clamp(Point.Y, Box.Min.Y, Box.Max.Y);
	}
	else if(IsOnYWall){
		const double MinDiff = abs(Point.Y - Box.Min.Y);
		const double MaxDiff = abs(Point.Y - Box.Max.Y);
		Point.Y = MinDiff < MaxDiff ? Box.Min.Y : Box.Max.Y;

		Point.X = FMath::Clamp(Point.X, Box.Min.X, Box.Max.X);
	}

	if (Point.Z < Box.Min.Z)
	{
		Point.Z = Box.Min.Z;
	}
	else if (Point.Z > Box.Max.Z)
	{
		Point.Z = Box.Max.Z;
	}

	DoorSpawn.SetLocation(Point);
}

void ARoomBounds::ManageDoor(){
	FReberuDoor* CurrentDoor = Room.GetDoorById(CurrentlyEditingDoorId);

	if(!CurrentDoor) return;
	
	CurrentDoor->DoorTransform = DoorSpawn;
}
