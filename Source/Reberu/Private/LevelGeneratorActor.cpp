// Copyright Peter Gilbert, All Rights Reserved


#include "LevelGeneratorActor.h"

#include "Reberu.h"
#include "RoomBounds.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Data/ReberuData.h"
#include "Data/ReberuRoomData.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ALevelGeneratorActor::ALevelGeneratorActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Referenced: https://unrealcommunity.wiki/add-in-editor-icon-to-your-custom-actor-vdxl1p27 for custom icon
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneComponent->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneComponent);

	bReplicates = true;

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
            : NoteTextureObject(TEXT("/Engine/EditorResources/S_TriggerBox"))
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
		SpriteComponent->Sprite = SpriteTexture ? SpriteTexture : ConstructorStatics.NoteTextureObject.Get();
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

ULevelStreamingDynamic* ALevelGeneratorActor::SpawnRoom(UReberuRoomData* InRoom, const FTransform& SpawnTransform, FString LevelName){
	if(!GetWorld() || !InRoom) return nullptr;

	bool bSpawnedSuccessfully = false;

	// Check if there is already a level with this name:
	if (LocalSpawnedLevels.Contains(LevelName)) {return nullptr;}
	
	ULevelStreamingDynamic* SpawnedRoom = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, InRoom->Room.Level, SpawnTransform,
		bSpawnedSuccessfully, LevelName);

	if(!bSpawnedSuccessfully) REBERU_LOG_ARGS(Warning, "SpawnRoom failed spawning room with name %s %d", *InRoom->RoomName.ToString(), HasAuthority())

	if(HasAuthority()){
		SpawnedRoomLevels.Add(FRoomLevel(InRoom, SpawnTransform, LevelName));
	}
	
	return SpawnedRoom;
}

void ALevelGeneratorActor::DespawnRoom(ULevelStreamingDynamic* SpawnedRoom){
	SpawnedRoom->SetIsRequestingUnloadAndRemoval(true);
}

AActor* ALevelGeneratorActor::SpawnDoor(UReberuData* ReberuData, ARoomBounds* TargetRoomBounds, FString DoorId, bool bIsOrphaned){
	if(DoorId.IsEmpty()) return nullptr;
	
	FReberuDoor ReberuDoor = *TargetRoomBounds->Room.GetDoorById(DoorId);
	if(!HasAuthority()) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FTransform LastRoomTransform = TargetRoomBounds->GetActorTransform();
	FVector RotatedFromDoorExtent = UKismetMathLibrary::Quat_RotateVector(
		ReberuDoor.DoorTransform.GetRotation(),
		FVector(ReberuDoor.BoxExtent.X, 0.f, -ReberuDoor.BoxExtent.Z));
	ReberuDoor.DoorTransform.SetLocation(ReberuDoor.DoorTransform.GetLocation() + RotatedFromDoorExtent);
	FTransform LastRoomDoorTransform = ReberuDoor.DoorTransform * LastRoomTransform;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FReberuDoorInfo* DoorInfo = nullptr;
	if (ReberuDoor.DoorTag.IsValid()){
		DoorInfo = ReberuData->DoorMap.Find(ReberuDoor.DoorTag);
	}
	else if (ReberuData->DoorMap.Contains(ReberuEmptyDoorTag)){
		DoorInfo = ReberuData->DoorMap.Find(ReberuEmptyDoorTag);
	}
	else{
		return nullptr;
	}

	if(!DoorInfo){
		REBERU_LOG_ARGS(Warning, "No door found in door map with the tag: %s", *ReberuDoor.DoorTag.ToString())
		return nullptr;
	}

	if(!bIsOrphaned && !DoorInfo->DoorActor->IsValidLowLevel()){
		REBERU_LOG_ARGS(Warning, "No door actor found in door map with the tag: %s", *ReberuDoor.DoorTag.ToString())
		return nullptr;
	}

	if(bIsOrphaned && !DoorInfo->BlockedDoorActor->IsValidLowLevel()){
		REBERU_LOG_ARGS(Warning, "No blocked door found in door map with the tag: %s", *ReberuDoor.DoorTag.ToString())
		return nullptr;
	}

	TSubclassOf<AActor> DoorActorClass = bIsOrphaned ? DoorInfo->BlockedDoorActor : DoorInfo->DoorActor;
	
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* SpawnedDoor = World->SpawnActor<AActor>(DoorActorClass, LastRoomDoorTransform, SpawnParams);

	return SpawnedDoor;
}

void ALevelGeneratorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALevelGeneratorActor, SpawnedRoomLevels);
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

bool ALevelGeneratorActor::BacktrackSourceRoom(TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode*& SourceRoomNode, const ERoomBacktrack BacktrackMethod){
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

void ALevelGeneratorActor::ChooseTargetRoom(TArray<UReberuRoomData*>& TargetRoomChoices, UReberuData* ReberuData, FReberuMove& SourceMove, FReberuMove& NewMove){
}

void ALevelGeneratorActor::ChooseSourceDoor(TArray<FReberuDoor>& SourceRoomDoorChoices, UReberuData* ReberuData, FReberuMove& SourceMove){
}

void ALevelGeneratorActor::ChooseTargetDoor(TArray<FReberuDoor>& SourceRoomDoorChoices, UReberuData* ReberuData, FReberuMove& SourceMove, FReberuDoor& SourceDoor,
	UReberuRoomData* TargetRoom){
}

void ALevelGeneratorActor::PostProcessing(UReberuData* ReberuData){
	K2_PostProcessing(ReberuData);
}

bool ALevelGeneratorActor::PlaceNextRoom(UReberuData* ReberuData, FReberuMove& SourceMove, FReberuMove& NewMove){
	REBERU_LOG(Log, "Trying to place next room...")

	NewMove.SourceRoomBounds = SourceMove.TargetRoomBounds;

	// Get all choices for everything. Could definitely make this more efficient.

	// Get source room possible doors (remove the already used doors)
	TArray<FReberuDoor> SourceRoomDoorChoices;
	for(FReberuDoor Door: SourceMove.TargetRoomBounds->Room.ReberuDoors){
		if(!SourceMove.TargetRoomBounds->Room.UsedDoors.Contains(Door.DoorId)){
			SourceRoomDoorChoices.Add(Door);
		}
	}
	ChooseSourceDoor(SourceRoomDoorChoices, ReberuData, SourceMove);

	// get target room possibilities
	TArray<UReberuRoomData*> TargetRoomChoices;
	for(UReberuRoomData* TargetRoom : ReberuData->ReberuRooms){
		if(SourceMove.RoomData->Room.bAllowSameRoomConnect == false && SourceMove.RoomData == TargetRoom){
			continue;
		}
		TargetRoomChoices.Add(TargetRoom);
	}
	ChooseTargetRoom(TargetRoomChoices, ReberuData, SourceMove, NewMove);
	
	TArray<FAttemptedMove> PossibleMoves;
	
	// Generate possible moves and then delete them based on the already created attempted moves...
	// TODO get rid of the nested loop bc this is pretty gross. O(n^3)
	for(FReberuDoor SourceDoor : SourceRoomDoorChoices){
		for(UReberuRoomData* TargetRoom : TargetRoomChoices){
			TArray<FReberuDoor> TargetRoomDoorChoices = TargetRoom->Room.ReberuDoors;
			ChooseTargetDoor(TargetRoomDoorChoices, ReberuData, SourceMove, SourceDoor, TargetRoom);
			
			for(FReberuDoor TargetDoor : TargetRoomDoorChoices){
				FAttemptedMove PossibleMove = FAttemptedMove(TargetRoom, SourceDoor.DoorId, TargetDoor.DoorId);
				if(!SourceMove.AttemptedMoves.Contains(PossibleMove)){
					if(SourceDoor.bOnlyConnectSameDoor || TargetDoor.bOnlyConnectSameDoor){
						if(SourceDoor.DoorTag.MatchesTagExact(TargetDoor.DoorTag) || (SourceDoor.DoorTag.IsValid() == false && TargetDoor.DoorTag.IsValid() == false)){
							PossibleMoves.Add(PossibleMove);
						}
					}
					else{
						PossibleMoves.Add(PossibleMove);
					}
				}
			}
		}
	}

	if(PossibleMoves.Num() == 0){
		REBERU_LOG(Log, "No more possible moves on this source room.")
		return false;
	}

	FAttemptedMove ChosenMove = GetRandomObjectInArray(PossibleMoves, ReberuRandomStream);
	SourceMove.AttemptedMoves.Add(ChosenMove);

	NewMove.RoomData = ChosenMove.RoomData;
	NewMove.SourceRoomDoor = ChosenMove.SourceRoomDoor;
	NewMove.TargetRoomDoor = ChosenMove.TargetRoomDoor;

	REBERU_LOG_ARGS(Log, "Trying : Source Room [%s] Source Door [%s] Target Room [%s] Target Door [%s]", *SourceMove.RoomData->RoomName.ToString(), *NewMove.SourceRoomDoor,
		*NewMove.RoomData->RoomName.ToString(), *NewMove.TargetRoomDoor)
	
	FReberuDoor SourceDoor = *SourceMove.RoomData->Room.GetDoorById(NewMove.SourceRoomDoor);
	FReberuDoor TargetDoor = *NewMove.RoomData->Room.GetDoorById(NewMove.TargetRoomDoor);

	const FTransform TargetRoomTransform = CalculateTransformFromDoor(SourceMove.TargetRoomBounds,
		SourceDoor, NewMove.RoomData, TargetDoor);

	ARoomBounds* TargetRoomBounds = SpawnRoomBounds(NewMove.RoomData, TargetRoomTransform);

	REBERU_LOG_ARGS(Log, "Spawned in New room bounds, %s (%s), which is connected to: %s", *TargetRoomBounds->GetName(), *NewMove.RoomData->RoomName.ToString(), *SourceMove.TargetRoomBounds->GetName())
	
	// Check collision
	UWorld* World = GetWorld();
	if(!World) return false;
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TargetRoomBounds);

	TArray<FOverlapResult> Overlaps;
	
	TSet<AActor*> OverlappingActors;
	
	World->OverlapMultiByObjectType(Overlaps, TargetRoomBounds->RoomBox->GetCenterOfMass(), TargetRoomBounds->GetActorRotation().Quaternion(), ObjectParams, FCollisionShape::MakeBox(TargetRoomBounds->RoomBox->GetUnscaledBoxExtent()), Params);

	for (FOverlapResult& Overlap : Overlaps){
		OverlappingActors.Add(Overlap.GetActor());
	}
	
	REBERU_LOG_ARGS(Log, "Number of overlapping actors for %s is: %d", *TargetRoomBounds->GetName(), OverlappingActors.Num())

	for (AActor* OverlappedActor : OverlappingActors){
		REBERU_LOG_ARGS(Verbose, "Found overlapping Actor on %s : %s", *TargetRoomBounds->GetName(), *OverlappedActor->GetName())
	}
	
	if(OverlappingActors.Num() == 0){
		NewMove.TargetRoomBounds = TargetRoomBounds;
		// DrawDebugBox(World, TargetRoomBounds->RoomBox->GetCenterOfMass(), TargetRoomBounds->RoomBox->GetUnscaledBoxExtent(), TargetRoomBounds->GetActorRotation().Quaternion(), FColor::Green, true, -1, 0, 2.f);
		return true;
	}
		
	TargetRoomBounds->Destroy();
	return PlaceNextRoom(ReberuData, SourceMove, NewMove);
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
	else{
		return;
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
	SpawnedRoomLevels.Empty();
}

void ALevelGeneratorActor::OnRep_SpawnedRoomLevels(){
	if(HasAuthority()) return;

	TSet<FString> LevelNames;

	// Add new ones
	for (auto& [InRoom, SpawnTransform, LevelName] : SpawnedRoomLevels){
		LevelNames.Add(LevelName);
		
		if(ULevelStreamingDynamic* NewRoom = SpawnRoom(InRoom, SpawnTransform, LevelName)){
			LocalSpawnedLevels.Add(LevelName, NewRoom);
		}
	}
	
	// Clear previous levels
	for (TTuple<FString, ULevelStreamingDynamic*> Level : LocalSpawnedLevels){
		if(!LevelNames.Contains(Level.Key)){
			Level.Value->SetIsRequestingUnloadAndRemoval(true);
		}
	}
}



