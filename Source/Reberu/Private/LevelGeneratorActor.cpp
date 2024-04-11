// Copyright Peter Gilbert, All Rights Reserved


#include "LevelGeneratorActor.h"

#include "Reberu.h"
#include "RoomBounds.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Data/ReberuData.h"
#include "Data/ReberuRoomData.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ALevelGeneratorActor::ALevelGeneratorActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Referenced: https://unrealcommunity.wiki/add-in-editor-icon-to-your-custom-actor-vdxl1p27 for custom icon
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneComponent->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneComponent);

	// Structure to hold one-time initialization
    struct FConstructorStatics
    {
        // A helper class object we use to find target UTexture2D object in resource package
        ConstructorHelpers::FObjectFinderOptional<UTexture2D> NoteTextureObject;

        // Icon sprite category name
        FName ID_Notes;

        // Icon sprite display name
        FText NAME_Notes;

        FConstructorStatics()
            // Use helper class object to find the texture
            // "/Engine/EditorResources/S_Note" is resource path
            : NoteTextureObject(TEXT("/Engine/EditorResources/S_Note"))
            , ID_Notes(TEXT("Notes"))
            , NAME_Notes(NSLOCTEXT("SpriteCategory", "Notes", "Notes"))
        {
        }
    };
    static FConstructorStatics ConstructorStatics;

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->Sprite = ConstructorStatics.NoteTextureObject.Get();
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Notes;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Notes;
		SpriteComponent->Mobility = EComponentMobility::Static;
		SpriteComponent->SetupAttachment(RootComponent);
	}
#endif // WITH_EDITORONLY_DATA
}

void ALevelGeneratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	if(bStartOnBeginPlay){
		StartGeneration();
	}
}

bool ALevelGeneratorActor::CanStartGeneration() const{
	if(!GetWorld()){
		REBERU_LOG(Error, "Can't start generation because there is no world!!")
		return false;
	}
	
	if(!ReberuData){
		REBERU_LOG(Warning, "No Reberu data was supplied so we can't start generation!")
		return false;
	}

	if(bIsGenerating){
		REBERU_LOG(Warning, "Level generation has already begun, we can't start again!")
		return false;
	}

	if(ReberuData->ReberuRooms.IsEmpty()){
		REBERU_LOG(Warning, "Can't start Reberu generation -> supplied room list was empty!")
		return false;
	}

	if(!RoomBoundsClass->IsValidLowLevelFast()){
		REBERU_LOG(Warning, "Can't start Reberu generation -> No RoomBounds class was supplied!")
		return false;
	}
	return true;
}

TOptional<FReberuDoor> ALevelGeneratorActor::ChooseRoomDoor(const UReberuRoomData* InRoom) const{
	return GetRandomDoor(InRoom);
}

UReberuRoomData* ALevelGeneratorActor::ChooseNextRoom() const{
	return GetRandomRoom();
}

TOptional<FReberuDoor> ALevelGeneratorActor::GetRandomDoor(const UReberuRoomData* InRoom) const{
	if (InRoom->Room.ReberuDoors.IsEmpty()) return TOptional<FReberuDoor>();
	return InRoom->Room.ReberuDoors[ReberuRandomStream.RandRange(0, InRoom->Room.ReberuDoors.Num() - 1)];
}

UReberuRoomData* ALevelGeneratorActor::GetRandomRoom() const{
	if(!ReberuData || ReberuData->ReberuRooms.IsEmpty()) return nullptr;
	return ReberuData->ReberuRooms[ReberuRandomStream.RandRange(0, ReberuData->ReberuRooms.Num() - 1)];
}

ULevelStreamingDynamic* ALevelGeneratorActor::SpawnRoom(const UReberuRoomData* InRoom, const FTransform& SpawnTransform){
	if(!GetWorld() || !InRoom) return nullptr;

	bool bSpawnedSuccessfully = false;
	ULevelStreamingDynamic* SpawnedRoom = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, InRoom->Room.Level, SpawnTransform,
		bSpawnedSuccessfully, InRoom->RoomName.ToString());

	if(!bSpawnedSuccessfully) REBERU_LOG_ARGS(Warning, "SpawnRoom failed spawning room with name %s", InRoom->RoomName)

	return SpawnedRoom;
}

void ALevelGeneratorActor::DespawnRoom(ULevelStreamingDynamic* SpawnedRoom){
	SpawnedRoom->SetIsRequestingUnloadAndRemoval(true);
}

FTransform ALevelGeneratorActor::CalculateTransformFromDoor(ARoomBounds* CurrentRoomBounds, FReberuDoor CurrentRoomChosenDoor, UReberuRoomData* NextRoom, FReberuDoor NextRoomChosenDoor){

	FTransform CalculatedTransform = FTransform::Identity;

	//https://forums.unrealengine.com/t/how-to-rotate-around-an-arbitrary-point/283132/5
	
	// Get the location of the last room chosen door in world space
	FTransform LastRoomTransform = CurrentRoomBounds->GetActorTransform();
	FTransform LastRoomDoorTransform = CurrentRoomChosenDoor.DoorTransform * LastRoomTransform;

	// Get the location of the next room chosen door in world space
	FTransform NextRoomTransform = NextRoom->Room.BoxActorTransform;
	
	// https://forums.unrealengine.com/t/how-to-get-an-angle-between-2-vectors/280850/40

	FVector FromDoorForwardVector = CurrentRoomBounds->GetActorForwardVector();
	FromDoorForwardVector = UKismetMathLibrary::Quat_RotateVector(CurrentRoomChosenDoor.DoorTransform.GetRotation(), FromDoorForwardVector);
	
	FVector ToDoorForwardVector = NextRoom->Room.BoxActorTransform.GetUnitAxis( EAxis::X );
	ToDoorForwardVector = UKismetMathLibrary::Quat_RotateVector(NextRoomChosenDoor.DoorTransform.GetRotation(), ToDoorForwardVector);

	REBERU_LOG_ARGS(Warning, "from door rot %s, to door rot %s", *CurrentRoomChosenDoor.DoorTransform.GetRotation().Rotator().ToString(), *NextRoomChosenDoor.DoorTransform.GetRotation().Rotator().ToString())
	
	REBERU_LOG_ARGS(Warning, "forward vector 1 %s, forward vector 2 %s", *FromDoorForwardVector.ToString(), *ToDoorForwardVector.ToString())

	float AngleInDegrees = FMath::UnwindDegrees(180 + FMath::RadiansToDegrees(FMath::Acos(FromDoorForwardVector.GetSafeNormal().Dot(ToDoorForwardVector.GetSafeNormal()))));
	FRotator ToRotateBy = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(0.f, 0.f, 1.f), AngleInDegrees);

	REBERU_LOG_ARGS(Warning, "angle diff of %f, rotating by %s", AngleInDegrees, *ToRotateBy.ToString())
	
	CalculatedTransform.SetRotation(UKismetMathLibrary::ComposeRotators(ToRotateBy, NextRoomTransform.Rotator()).Quaternion());
	
	CalculatedTransform.SetLocation(LastRoomDoorTransform.GetLocation());

	FVector FinalLocation = CalculatedTransform.TransformPosition(NextRoomChosenDoor.DoorTransform.GetLocation());
	UKismetSystemLibrary::DrawDebugSphere(this, FinalLocation, 10, 12, FLinearColor::Yellow, 25, 2);
	FinalLocation = CalculatedTransform.GetLocation() + (CalculatedTransform.GetLocation() - FinalLocation);
	CalculatedTransform.SetLocation(FinalLocation);
	UKismetSystemLibrary::DrawDebugSphere(this, FinalLocation, 10, 12, FLinearColor::Red, 25, 2);
	return CalculatedTransform;
}

ARoomBounds* ALevelGeneratorActor::SpawnRoomBounds(const UReberuRoomData* InRoom, const FTransform& AtTransform){
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoomBounds* SpawnedBounds = World->SpawnActor<ARoomBounds>(RoomBoundsClass, AtTransform, SpawnParams);
	SpawnedBounds->Room = InRoom->Room;
	SpawnedBounds->RoomBox->SetBoxExtent(InRoom->Room.BoxExtent);
	return SpawnedBounds;
}

void ALevelGeneratorActor::PlaceNextRoom(){
	TArray<FReberuDoor> AttemptedCurrentRoomDoors;
	TArray<UReberuRoomData*> AttemptedNextRoomTypes;
	
	// Get door on current room
	TOptional<FReberuDoor> FromDoor = ChooseRoomDoor(MovesList.GetTail()->GetValue().RoomData);
	if (!FromDoor.IsSet()){
		REBERU_LOG_ARGS(Warning, "Generation stopped because FromDoor is not valid on %s", *MovesList.GetTail()->GetValue().RoomData->RoomName.ToString())
		return;
	}

	// Try to determine next room
	UReberuRoomData* NextRoomData = ChooseNextRoom();
	if (!NextRoomData){
		REBERU_LOG_ARGS(Warning, "Generation stopped because there is no NextRoom to select on %s", *MovesList.GetTail()->GetValue().RoomData->RoomName.ToString())
		return;
	}

	// Try to determine next room door
	TOptional<FReberuDoor> ToDoor = ChooseRoomDoor(NextRoomData);
	if (!FromDoor.IsSet()){
		REBERU_LOG_ARGS(Warning, "Generation stopped because ToDoor is not valid on %s", *NextRoomData->RoomName.ToString())
		return;
	}

	REBERU_LOG_ARGS(Log, "Old Door [%s] New Room [%s] New Door [%s]", *FromDoor->DoorId, *NextRoomData->RoomName.ToString(), *ToDoor->DoorId)

	const FTransform NewRoomTransform = CalculateTransformFromDoor(MovesList.GetTail()->GetValue().ToRoomBounds,
		FromDoor.GetValue(), NextRoomData, ToDoor.GetValue());

	ARoomBounds* NewRoomBounds = SpawnRoomBounds(NextRoomData, NewRoomTransform);
	MovesList.AddTail(FReberuMove(NextRoomData, NewRoomTransform, NewRoomBounds, true));
}

void ALevelGeneratorActor::StartGeneration(){
	if(!CanStartGeneration()) return;

	REBERU_LOG_ARGS(Log, "Starting level generation with %s", *ReberuData->GetName())

	bIsGenerating = true;

	ReberuRandomStream = ReberuSeed != -1 ? FRandomStream(ReberuSeed) : FRandomStream();

	UReberuRoomData* StartingRoomData = ReberuData->StartingRoom ? ReberuData->StartingRoom : GetRandomRoom();

	if(!StartingRoomData) return;
	
	ARoomBounds* StartingBounds = SpawnRoomBounds(StartingRoomData, FTransform::Identity);
	MovesList.AddHead(FReberuMove(StartingRoomData, StartingBounds->GetActorTransform(), StartingBounds, false));

	while (MovesList.Num() < ReberuData->TargetRoomAmount && bIsGenerating){
		PlaceNextRoom();
	}

	// Get door on current room
	// try to determine next room and also the next door
	// calculate transform for new room that is rotated appropriately
	// do collision test
	// if failure go back to the start of this
	// otherwise continue on

	// need to just refactor the stuff underneath here.
	// ULevelStreamingDynamic* StartingRoomInstance = SpawnRoom(StartingRoomData, FTransform::Identity);
	// if(!StartingRoomInstance) return;
	// LevelInstances.Add(StartingRoomInstance);
	//
	// UReberuRoomData* CurrentRoomData = StartingRoomData;
	//
	// while(LevelInstances.Num() < ReberuData->TargetRoomAmount && bIsGenerating){
	// 	FReberuDoor CurrentRoomChosenDoor = ChooseRoomDoor(CurrentRoomData);
	// 	UReberuRoomData* NextRoomData = ChooseNextRoom();
	// 	FReberuDoor NextRoomDoor = ChooseRoomDoor(NextRoomData);
	// 	FTransform NextRoomTransform = CalculateTransformFromDoor(
	// 		CurrentRoomData, CurrentRoomChosenDoor,
	// 		NextRoomData, NextRoomDoor);
	// 	ULevelStreamingDynamic* NewRoomInstance = SpawnRoom(NextRoomData, NextRoomTransform);
	// 	if(!NewRoomInstance) return;
	// 	LevelInstances.Add(NewRoomInstance);
	// 	CurrentRoomData = NextRoomData;
	// }
	
}

void ALevelGeneratorActor::ClearGeneration(){
	for (ULevelStreamingDynamic* LevelInstance : LevelInstances){
		if(LevelInstance->IsValidLowLevelFast()){
			DespawnRoom(LevelInstance);
		}
	}
	for (auto Move : MovesList){
		if(Move.ToRoomBounds){
			Move.ToRoomBounds->Destroy();
		}
	}
	bIsGenerating = false;
	// TODO reset all variables here so we can do another clean run. Should also call this in StartGeneration if we have already one one that still exists.
	MovesList.Empty();
	LevelInstances.Empty();
}





