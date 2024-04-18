// Copyright Peter Gilbert, All Rights Reserved


#include "Task/GenerateRoomsTask.h"

#include "Reberu.h"
#include "RoomBounds.h"

void FGenerateRoomsAction::UpdateOperation(FLatentResponse& Response){
	// todo do some failure checks here.

	// If we hit completed up here that means we've failed.
	const bool bHasFailed = !(LevelGenerator && ReberuData && ReberuData->ReberuRooms.Num());
	if(bIsCompleted || bHasFailed){
		REBERU_LOG(Error, "GenerateRoomsTask has failed!")
		Output = EGenerateRoomsOutputPins::OnFailed;
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}
	
	if(bWantToCancel){
		bSuccess = true;
		REBERU_LOG(Warning, "Received cancel request during Reberu generation. Clearing and cancelling...")
		LevelGenerator->ClearGeneration();
		Output = EGenerateRoomsOutputPins::OnCancelled;
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}
	
	if(bIsFirstCall){
		bIsFirstCall = false;
		bSuccess = true;

		REBERU_LOG_ARGS(Log, "Starting level generation with %s", *ReberuData->GetName())

		UReberuRoomData* StartingRoomData = ReberuData->StartingRoom ? ReberuData->StartingRoom : GetRandomObjectInArray<UReberuRoomData*>(ReberuData->ReberuRooms, ReberuRandomStream);
	
		ARoomBounds* StartingBounds = LevelGenerator->SpawnRoomBounds(StartingRoomData, FTransform::Identity);
		MovesList.AddHead(FReberuMove(StartingRoomData, StartingBounds->GetActorTransform(), StartingBounds, false));
		SourceRoomNode = MovesList.GetHead();
		
		// Trigger on started pin
		Output = EGenerateRoomsOutputPins::OnStarted;
		Response.TriggerLink(LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}

	if(MovesList.Num() < ReberuData->TargetRoomAmount && LevelGenerator->IsGenerating()){
		TSet<FString> AttemptedNewRoomDoors;
		TSet<FString> AttemptedOldRoomDoors;
		FReberuMove NewMove;
		ARoomBounds* FromRoomBounds = SourceRoomNode->GetValue().ToRoomBounds;

		// Try placing the next room
		const bool bRoomCreated = LevelGenerator->PlaceNextRoom(ReberuData, NewMove, FromRoomBounds, AttemptedNewRoomDoors, AttemptedNewRooms, AttemptedOldRoomDoors);
		AttemptedNewRooms.Empty();

		if(!LevelGenerator->IsGenerating()){
			bWantToCancel = true;
			return;
		}

		// If we created a room successfully, update values accordingly
		if(bRoomCreated){
			MaxBacktrackTries = ReberuData->MaxBacktrackTries;
			NewMove.FromRoomBounds->Room.UsedDoors.Add(NewMove.FromRoomDoor);
			NewMove.ToRoomBounds->Room.UsedDoors.Add(NewMove.ToRoomDoor);
			MovesList.AddTail(NewMove);
			REBERU_LOG(Log, "Added new move to the list!")
			// Choose the next source room (or keep the current one if applicable)
			LevelGenerator->ChooseSourceRoom(SourceRoomNode, ReberuData->RoomSelectionMethod);

			// Execute OnRoomPlaced pin
			Output = EGenerateRoomsOutputPins::OnRoomPlaced;
			Response.TriggerLink(LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		}
		else{
			REBERU_LOG(Log, "Failed to place room during reberu generation. Trying to choose next room or backtrack...")
			// If we failed to place a room, try moving forward through the moveslist, if we're already at the tail, backtrack.
			// if we successfully choose a new room, we're done here.
			if(LevelGenerator->ChooseSourceRoom(SourceRoomNode, ReberuData->RoomSelectionMethod, true)) return;
			
			// backtrack here if possible, otherwise we fail
			if(MaxBacktrackTries > 0){
				MaxBacktrackTries--;
				const bool bBacktrackResult = LevelGenerator->BacktrackSourceRoom(SourceRoomNode, ReberuData->BacktrackMethod, AttemptedNewRooms);
				if(!bBacktrackResult) REBERU_LOG(Warning, "We failed to backtrack, worth debugging!")
			}
			else{
				LevelGenerator->SetIsGenerating(false);
				bIsCompleted = true;
				return;
			}
		}
	}
	else{
		bIsCompleted = true;
	}

	
	// Do OnCompleted here!
	if(bIsCompleted){
		REBERU_LOG_ARGS(Log, "Reberu Generation complete! Created %d rooms!", MovesList.Num())
		Output = EGenerateRoomsOutputPins::OnCompleted;
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
	}
}
