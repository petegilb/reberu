﻿// Copyright Peter Gilbert, All Rights Reserved


#include "Task/GenerateRoomsTask.h"

#include "Reberu.h"
#include "RoomBounds.h"

void FGenerateRoomsAction::UpdateOperation(FLatentResponse& Response){

	// Delay for visual debugging purposes. False by default.
	if(bDebugDelay){
		DebugTimer += Response.ElapsedTime();
		if(DebugTimer < .5f){
			return;
		}
		DebugTimer = 0.f;
	}

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

		/** Check for duplicate door ids just in case. */
		TMap<FString, UReberuRoomData*> DoorToRoomMap; 
		for(UReberuRoomData* Room : ReberuData->ReberuRooms){
			for(const FReberuDoor& Door : Room->Room.ReberuDoors){

				if(Door.DoorTransform.GetScale3D() != FVector::One()){
					REBERU_LOG_ARGS(Error, "Detected door %s on room %s with scale != 1 in their transform. Will cause door visual to look off.", *Room->RoomName.ToString(), *Door.DoorId)
				}
				
				if(DoorToRoomMap.Contains(Door.DoorId)){
					const UReberuRoomData* OldRoom = *DoorToRoomMap.Find(Door.DoorId);
					REBERU_LOG_ARGS(Warning, "Duplicate Door Id detected in %s with id: %s (other in %s). Please regenerate ids to have unique door ids or it may cause unexpected issues.",
						*Room->RoomName.ToString(), *Door.DoorId, *OldRoom->RoomName.ToString())
				}
				else{
					DoorToRoomMap.Add(Door.DoorId, Room);
				}
			}
		}

		if(Seed > 0){
			ReberuRandomStream = FRandomStream(Seed);
		}
		else{
			ReberuRandomStream.GenerateNewSeed();
		}

		UReberuRoomData* StartingRoomData = ReberuData->StartingRoom ? ReberuData->StartingRoom : GetRandomObjectInArray<UReberuRoomData*>(ReberuData->ReberuRooms, ReberuRandomStream);
	
		ARoomBounds* StartingBounds = LevelGenerator->SpawnRoomBounds(StartingRoomData, StartRoomTransform);
		MovesList.AddHead(FReberuMove(StartingRoomData, StartingBounds->GetActorTransform(), StartingBounds, false));
		SourceRoomNode = MovesList.GetHead();
		
		// Trigger on started pin
		Output = EGenerateRoomsOutputPins::OnStarted;
		Response.TriggerLink(LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}

	if(MovesList.Num() < ReberuData->TargetRoomAmount && LevelGenerator->IsGenerating()){
		FReberuMove NewMove;

		// Try placing the next room
		const bool bRoomCreated = LevelGenerator->PlaceNextRoom(ReberuData, SourceRoomNode->GetValue(), NewMove);

		if(!LevelGenerator->IsGenerating()){
			bWantToCancel = true;
			return;
		}

		// If we created a room successfully, update values accordingly
		if(bRoomCreated){
			MaxBacktrackTries = ReberuData->MaxBacktrackTries;
			NewMove.SourceRoomBounds->Room.UsedDoors.Add(NewMove.SourceRoomDoor);
			NewMove.TargetRoomBounds->Room.UsedDoors.Add(NewMove.TargetRoomDoor);
			// for bfs/dfs i think this should work since we are going in order but it won't for custom methods
			NewMove.TargetRoomBounds->Room.Depth = NewMove.SourceRoomBounds->Room.Depth + 1; 
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
			REBERU_LOG_ARGS(Warning, "Max backtrack tries is %d", MaxBacktrackTries)
			if(MaxBacktrackTries > 0){
				MaxBacktrackTries--;
				const bool bBacktrackResult = LevelGenerator->BacktrackSourceRoom(SourceRoomNode, ReberuData->BacktrackMethod);
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
		const bool PreProcessingResult = LevelGenerator->PreProcessing(ReberuData);
		if (!PreProcessingResult){
			Output = EGenerateRoomsOutputPins::OnFailed;
			Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
			return;
		}
		Output = EGenerateRoomsOutputPins::OnCompleted;
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
	}
}
