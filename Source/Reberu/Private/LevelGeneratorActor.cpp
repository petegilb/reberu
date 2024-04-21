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
#endif
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

	if(bIsGenerating){
		REBERU_LOG(Warning, "Level generation has already begun, we can't start again!")
		return false;
	}

	if(!RoomBoundsClass->IsValidLowLevelFast()){
		REBERU_LOG(Warning, "Can't start Reberu generation -> No RoomBounds class was supplied!")
		return false;
	}
	return true;
}

ULevelStreamingDynamic* ALevelGeneratorActor::SpawnRoom(const UReberuRoomData* InRoom, const FTransform& SpawnTransform, FString LevelName){
	if(!GetWorld() || !InRoom) return nullptr;

	bool bSpawnedSuccessfully = false;
	ULevelStreamingDynamic* SpawnedRoom = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, InRoom->Room.Level, SpawnTransform,
		bSpawnedSuccessfully, LevelName);

	if(!bSpawnedSuccessfully) REBERU_LOG_ARGS(Warning, "SpawnRoom failed spawning room with name %s", InRoom->RoomName)
	
	return SpawnedRoom;
}

void ALevelGeneratorActor::DespawnRoom(ULevelStreamingDynamic* SpawnedRoom){
	SpawnedRoom->SetIsRequestingUnloadAndRemoval(true);
}

FTransform ALevelGeneratorActor::CalculateTransformFromDoor(ARoomBounds* SourceRoomBounds, FReberuDoor SourceRoomChosenDoor, UReberuRoomData* TargetRoom, FReberuDoor TargetRoomChosenDoor){

	FTransform CalculatedTransform = FTransform::Identity;

	// We also alter the door location by the extent of the door
	
	// Get the location of the last room chosen door in world space
	FTransform LastRoomTransform = SourceRoomBounds->GetActorTransform();
	FVector RotatedFromDoorExtent = UKismetMathLibrary::Quat_RotateVector(
		SourceRoomChosenDoor.DoorTransform.GetRotation(),
		FVector(SourceRoomChosenDoor.BoxExtent.X, 0.f, -SourceRoomChosenDoor.BoxExtent.Z));
	SourceRoomChosenDoor.DoorTransform.SetLocation(SourceRoomChosenDoor.DoorTransform.GetLocation() + RotatedFromDoorExtent);
	FTransform LastRoomDoorTransform = SourceRoomChosenDoor.DoorTransform * LastRoomTransform;

	// Get the location of the next room chosen door in world space
	FTransform TargetRoomTransform = TargetRoom->Room.BoxActorTransform;

	// Calculate Rotation
	// https://forums.unrealengine.com/t/how-to-get-an-angle-between-2-vectors/280850/39
	
	FVector FromDoorForwardVector = SourceRoomBounds->GetActorForwardVector();
	FromDoorForwardVector = UKismetMathLibrary::Quat_RotateVector(SourceRoomChosenDoor.DoorTransform.GetRotation(), FromDoorForwardVector);
	
	FVector ToDoorForwardVector = TargetRoom->Room.BoxActorTransform.GetUnitAxis( EAxis::X );
	ToDoorForwardVector = UKismetMathLibrary::Quat_RotateVector(TargetRoomChosenDoor.DoorTransform.GetRotation(), ToDoorForwardVector);

	REBERU_LOG_ARGS(Verbose, "from door rot %s, to door rot %s", *SourceRoomChosenDoor.DoorTransform.GetRotation().Rotator().ToString(), *TargetRoomChosenDoor.DoorTransform.GetRotation().Rotator().ToString())
	
	REBERU_LOG_ARGS(Verbose, "forward vector 1 %s, forward vector 2 %s", *FromDoorForwardVector.ToString(), *ToDoorForwardVector.ToString())
	
	float DotProduct = FromDoorForwardVector.GetSafeNormal().Dot(ToDoorForwardVector.GetSafeNormal());
	float DegreeDifference = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	REBERU_LOG_ARGS(Verbose, "Dot Product %f, Degree Difference %f", DotProduct, DegreeDifference)
	float AngleInDegrees = FMath::UnwindDegrees(180 - DegreeDifference);

	FVector CrossProduct = FVector::CrossProduct(FromDoorForwardVector.GetSafeNormal(), ToDoorForwardVector.GetSafeNormal());

	// If the Z component of the cross product is less than zero, the second vector is to the left of the first
	if (CrossProduct.Z < 0)
	{
		AngleInDegrees = -AngleInDegrees;
	}
	
	FRotator ToRotateBy = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(0.f, 0.f, 1.f), AngleInDegrees);

	REBERU_LOG_ARGS(Verbose, "angle diff of %f, rotating by %s", AngleInDegrees, *ToRotateBy.ToString())
	
	CalculatedTransform.SetRotation(UKismetMathLibrary::ComposeRotators(TargetRoomTransform.Rotator(), ToRotateBy).Quaternion());

	// Calculate Location
	
	CalculatedTransform.SetLocation(LastRoomDoorTransform.GetLocation());
	FVector RotatedToDoorExtent = UKismetMathLibrary::Quat_RotateVector(
		TargetRoomChosenDoor.DoorTransform.GetRotation(),
		FVector(TargetRoomChosenDoor.BoxExtent.X, 0.f, -TargetRoomChosenDoor.BoxExtent.Z));

	FVector TargetRoomChosenDoorAltered = TargetRoomChosenDoor.DoorTransform.GetLocation() + RotatedToDoorExtent;
	
	FVector FinalLocation = CalculatedTransform.TransformPosition(TargetRoomChosenDoorAltered);
	// UKismetSystemLibrary::DrawDebugSphere(this, FinalLocation, 10, 6, FLinearColor::Yellow, 25, 1);
	FinalLocation = LastRoomDoorTransform.GetLocation() + (LastRoomDoorTransform.GetLocation() - FinalLocation);
	CalculatedTransform.SetLocation(FinalLocation);
	// UKismetSystemLibrary::DrawDebugSphere(this, FinalLocation, 10, 6, FLinearColor::Red, 25, 1);
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

bool ALevelGeneratorActor::ChooseSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomSelection SelectionType, bool bFromError){
	// TODO implement other selection types

	const ARoomBounds* SourceRoomBounds = SourceRoomNode->GetValue().TargetRoomBounds;
	
	switch(SelectionType){
	case ERoomSelection::Breadth:
		// If we've used all doors on our current bounds, let's move to the next room
		// Place all that we can on the most recent room
		if(SourceRoomBounds->Room.UsedDoors.Num() == SourceRoomBounds->Room.ReberuDoors.Num() || bFromError){
			while(SourceRoomNode != MovesList.GetTail() && (SourceRoomNode->GetValue().TargetRoomBounds == SourceRoomBounds || SourceRoomNode->GetValue().TargetRoomBounds == nullptr)){
				SourceRoomNode = SourceRoomNode->GetNextNode();
				REBERU_LOG(Verbose, "Iterated here!")
			}
			// if the new room bounds is a different value compared to the initial version
			if(SourceRoomNode->GetValue().TargetRoomBounds != SourceRoomBounds){
				REBERU_LOG_ARGS(Log, "Changed rooms from %s (%s) -> %s (%s)",
					*SourceRoomBounds->GetName(), *SourceRoomNode->GetPrevNode()->GetValue().RoomData->RoomName.ToString(),
					*SourceRoomNode->GetValue().TargetRoomBounds->GetName(), *SourceRoomNode->GetValue().RoomData->RoomName.ToString())
				
				return true;
			}
			return false;
		}
		else{
			// if we made it here that means there are still unused rooms on our current source room
			REBERU_LOG(Log, "Kept going with the same room!")
			return true;
		}
		break;
	default: ;
	}
	
	
	return false;
}

bool ALevelGeneratorActor::BacktrackSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, ERoomBacktrack BacktrackMethod, TSet<UReberuRoomData*>& AttemptedNewRooms){
	REBERU_LOG(Log, "Trying to backtrack source room...")
	if(MovesList.Num() <= 1) return false;
	
	TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode* CurrentTail = MovesList.GetTail();

	switch(BacktrackMethod){
	case ERoomBacktrack::FromTail:
		if(SourceRoomNode == CurrentTail && SourceRoomNode->GetPrevNode()){
			// Set the source room to the previous node
			SourceRoomNode = SourceRoomNode->GetPrevNode();
			
			REBERU_LOG_ARGS(Log, "Backtracking from %s (%s) -> %s (%s)",
				*CurrentTail->GetValue().TargetRoomBounds->GetName(), *CurrentTail->GetValue().RoomData->RoomName.ToString(),
				*CurrentTail->GetValue().SourceRoomBounds->GetName(), *CurrentTail->GetPrevNode()->GetValue().RoomData->RoomName.ToString())
			
			// need to add to attemptednewrooms so we don't try the same room again
			AttemptedNewRooms.Add(CurrentTail->GetValue().RoomData);                      
			// Destroy the bounds that we are backtracking from
			CurrentTail->GetValue().TargetRoomBounds->Destroy();
			// update used doors on the bounds that we are backtracking to
			CurrentTail->GetValue().SourceRoomBounds->Room.UsedDoors.Remove(CurrentTail->GetValue().SourceRoomDoor);
			
			MovesList.RemoveNode(CurrentTail);
			return true;
		}
		break;
	default: ;
	}
	
	
	return false;
}

bool ALevelGeneratorActor::PlaceNextRoom(UReberuData* ReberuData, FReberuMove& NewMove, ARoomBounds* SourceRoomBounds, TSet<FString>& AttemptedNewRoomDoors,
                                         TSet<UReberuRoomData*>& AttemptedNewRooms, TSet<FString>& AttemptedOldRoomDoors)
{
	REBERU_LOG(Log, "Trying to place next room...")
	// using this bool to check whether or not we should recursively call this function if we've chosen a new room or door that requires it.
	bool bAltered = false;

	NewMove.SourceRoomBounds = SourceRoomBounds;

	// Get all choices for everything. Could definitely make this more efficient.
	TArray<UReberuRoomData*> TargetRoomChoices;
	for (UReberuRoomData* RoomOption : ReberuData->ReberuRooms){
		if(!AttemptedNewRooms.Contains(RoomOption)){
			TargetRoomChoices.Add(RoomOption);
		}
	}

	// if all the values we need are not valid, let's set them
	NewMove.RoomData = !NewMove.RoomData && TargetRoomChoices.Num() > 0 ? GetRandomObjectInArray<UReberuRoomData*>(TargetRoomChoices, ReberuRandomStream) : NewMove.RoomData;
	
	TArray<FReberuDoor> TargetRoomDoorChoices;
	if(NewMove.RoomData){
		for (FReberuDoor Door : NewMove.RoomData->Room.ReberuDoors){
			if(!AttemptedNewRoomDoors.Contains(Door.DoorId)){
				TargetRoomDoorChoices.Add(Door);
			}
		}
	}
	
	TArray<FReberuDoor> SourceRoomDoorChoices;
	for (FReberuDoor Door : SourceRoomBounds->Room.ReberuDoors){
		if(!(AttemptedOldRoomDoors.Contains(Door.DoorId) || SourceRoomBounds->Room.UsedDoors.Contains(Door.DoorId))){
			SourceRoomDoorChoices.Add(Door);
		}
	}

	// If all doors on the source room have already been used, exit and try a new room
	if(SourceRoomBounds->Room.UsedDoors.Num() == SourceRoomBounds->Room.ReberuDoors.Num()){
		REBERU_LOG(Log, "All doors on the source room have already been used.")
		return false;
	}

	NewMove.SourceRoomDoor = NewMove.SourceRoomDoor.IsEmpty() && SourceRoomDoorChoices.Num() > 0 ? GetRandomObjectInArray(SourceRoomDoorChoices, ReberuRandomStream).DoorId : NewMove.SourceRoomDoor;
	NewMove.TargetRoomDoor = NewMove.TargetRoomDoor.IsEmpty() && TargetRoomDoorChoices.Num() > 0 ? GetRandomObjectInArray(TargetRoomDoorChoices, ReberuRandomStream).DoorId : NewMove.TargetRoomDoor;
	
	// TODO add logic for transitions and rules

	// if they still aren't set, we've hit an error
	if(!(!NewMove.SourceRoomDoor.IsEmpty() && NewMove.RoomData && !NewMove.TargetRoomDoor.IsEmpty())){
		REBERU_LOG_ARGS(Warning, "Generation stopped because there was an unset value on initial room placement for room %s | %d | %d | %d",
			*SourceRoomBounds->GetName(), !NewMove.SourceRoomDoor.IsEmpty(), NewMove.RoomData, !NewMove.TargetRoomDoor.IsEmpty())
		bIsGenerating = false;
		return false;
	}
	
	// Try the doors on the new room first
	if(TargetRoomDoorChoices.Num() > 0){
		NewMove.TargetRoomDoor = GetRandomObjectInArray<FReberuDoor>(TargetRoomDoorChoices, ReberuRandomStream).DoorId;

		REBERU_LOG_ARGS(Log, "Trying out new door %s on room %s", *NewMove.TargetRoomDoor, *NewMove.RoomData->RoomName.ToString());
		AttemptedNewRoomDoors.Add(NewMove.TargetRoomDoor);
		bAltered = true;
	}

	// Then try selecting a new room if we didn't select a new door
	if(!bAltered){
		if(TargetRoomChoices.Num() > 0){
			NewMove.RoomData = GetRandomObjectInArray<UReberuRoomData*>(TargetRoomChoices, ReberuRandomStream);
			// reset attempted doors since we will be selecting a new room
			AttemptedNewRoomDoors.Empty();
			AttemptedNewRooms.Add(NewMove.RoomData);

			REBERU_LOG_ARGS(Log, "Trying out new room %s", *NewMove.RoomData->RoomName.ToString());

			return PlaceNextRoom(ReberuData, NewMove, SourceRoomBounds, AttemptedNewRoomDoors, AttemptedNewRooms, AttemptedOldRoomDoors);
		}
	}
	
	// if all that doesn't work, try selecting a new door on the old room and restarting from there
	if(!bAltered){
		if(SourceRoomDoorChoices.Num() > 0){
			NewMove.SourceRoomDoor = GetRandomObjectInArray(SourceRoomDoorChoices, ReberuRandomStream).DoorId;
			
			AttemptedNewRooms.Empty();
			AttemptedNewRoomDoors.Empty();
			AttemptedOldRoomDoors.Add(NewMove.SourceRoomDoor);

			REBERU_LOG_ARGS(Log, "Trying to select different door on the current room %s", *NewMove.SourceRoomDoor, *NewMove.RoomData->RoomName.ToString());
			
			return PlaceNextRoom(ReberuData, NewMove, SourceRoomBounds, AttemptedNewRoomDoors, AttemptedNewRooms, AttemptedOldRoomDoors);
		}
		
	}
	
	// Add to the used sets if they don't already exist in there
	if(!AttemptedNewRooms.Contains(NewMove.RoomData)) AttemptedNewRooms.Add(NewMove.RoomData);
	if(!AttemptedNewRoomDoors.Contains(NewMove.TargetRoomDoor)) AttemptedNewRoomDoors.Add(NewMove.TargetRoomDoor);
	if(!AttemptedOldRoomDoors.Contains(NewMove.SourceRoomDoor)) AttemptedOldRoomDoors.Add(NewMove.SourceRoomDoor);

	if(!bAltered){
		REBERU_LOG(Log, "Ran out of options trying to alter new room move during generation.")
		return false;
	}

	REBERU_LOG_ARGS(Log, "Old Door [%s] New Room [%s] New Door [%s]", *NewMove.SourceRoomDoor, *NewMove.RoomData->RoomName.ToString(), *NewMove.TargetRoomDoor)

	FReberuDoor FromDoor = *SourceRoomBounds->Room.GetDoorById(NewMove.SourceRoomDoor);
	FReberuDoor ToDoor = *NewMove.RoomData->Room.GetDoorById(NewMove.TargetRoomDoor);

	const FTransform NewRoomTransform = CalculateTransformFromDoor(SourceRoomBounds,
		FromDoor, NewMove.RoomData, ToDoor);

	ARoomBounds* NewRoomBounds = SpawnRoomBounds(NewMove.RoomData, NewRoomTransform);

	REBERU_LOG_ARGS(Log, "Spawned in New room bounds, %s (%s), which is connected to: %s", *NewRoomBounds->GetName(), *NewMove.RoomData->RoomName.ToString(), *SourceRoomBounds->GetName())
	
	// Check collision
	UWorld* World = GetWorld();
	if(!World) return false;
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(NewRoomBounds);

	TArray<FOverlapResult> Overlaps;
	
	TSet<AActor*> OverlappingActors;
	
	World->OverlapMultiByObjectType(Overlaps, NewRoomBounds->GetActorLocation(), FQuat::Identity, ObjectParams, FCollisionShape::MakeBox(NewRoomBounds->RoomBox->GetScaledBoxExtent()), Params);

	for (FOverlapResult& Overlap : Overlaps){
		OverlappingActors.Add(Overlap.GetActor());
	}
	
	REBERU_LOG_ARGS(Log, "Number of overlapping actors for %s is: %d", *NewRoomBounds->GetName(), OverlappingActors.Num())

	for (AActor* OverlappedActor : OverlappingActors){
		REBERU_LOG_ARGS(Verbose, "Found overlapping Actor on %s : %s", *NewRoomBounds->GetName(), *OverlappedActor->GetName())
	}
	
	if(OverlappingActors.Num() == 0){
		NewMove.TargetRoomBounds = NewRoomBounds;
		return true;
	}
		
	NewRoomBounds->Destroy();
	return PlaceNextRoom(ReberuData, NewMove, SourceRoomBounds, AttemptedNewRoomDoors, AttemptedNewRooms, AttemptedOldRoomDoors);
}

void ALevelGeneratorActor::StartGeneration(){
	if(!CanStartGeneration()) return;

	REBERU_LOG(Log, "Starting Reberu level generation")

	bIsGenerating = true;

	// Execute bp start generation event
	K2_StartGeneration();
}

void ALevelGeneratorActor::ClearGeneration(){
	if(UWorld* World = GetWorld()){
		World->GetLatentActionManager().RemoveActionsForObject(this);
	}
	for (const auto Move : MovesList){
		if(Move.TargetRoomBounds){
			Move.TargetRoomBounds->Destroy();
		}
		if(Move.SpawnedLevel){
			DespawnRoom(Move.SpawnedLevel);
		}
	}
	bIsGenerating = false;
	MovesList.Empty();
}





