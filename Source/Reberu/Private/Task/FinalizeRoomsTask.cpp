// Copyright Peter Gilbert, All Rights Reserved


#include "Task/FinalizeRoomsTask.h"
#include "Reberu.h"
#include "RoomBounds.h"

void FFinalizeRoomsTask::UpdateOperation(FLatentResponse& Response){
	
	if(bWantToCancel){
		bSuccess = true;
		REBERU_LOG(Warning, "Received cancel request during Reberu level instance placement. Clearing and cancelling...")
		LevelGenerator->ClearGeneration();
		Output = EFinalizeRoomsOutputPins::OnCancelled;
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}
	
	if(bIsFirstCall){
		bIsFirstCall = false;
		bSuccess = true;

		CurrentMove = MovesList.GetHead();
		
		// Trigger on started pin
		Output = EFinalizeRoomsOutputPins::OnStarted;
		Response.TriggerLink(LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}
	
	if(CurrentMove){
		REBERU_LOG(Log, "Creating level associated with room...")
		FTransform TempTransform = CurrentMove->GetValue().RoomData->Room.BoxActorTransform;
		TempTransform.SetLocation(-TempTransform.GetLocation());
		TempTransform.SetRotation(TempTransform.GetRotation().Inverse());
		const FTransform FinalTransform = TempTransform * CurrentMove->GetValue().TargetRoomBounds->GetActorTransform();
		CurrentMove->GetValue().SpawnedLevel = LevelGenerator->SpawnRoom(CurrentMove->GetValue().RoomData, FinalTransform,
		                                                                 CurrentMove->GetValue().RoomData->RoomName.ToString() + FString::FromInt(CurrentIdx));

		// Delete the room bounds associated with this new level.
		CurrentMove->GetValue().TargetRoomBounds->Destroy();

		if(CurrentMove->GetNextNode()){
			CurrentIdx++;
			CurrentMove = CurrentMove->GetNextNode();
		}
		else{
			CurrentMove = nullptr;
			bIsCompleted = true;
		}

		Output = EFinalizeRoomsOutputPins::OnLevelCreated;
		Response.TriggerLink(LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
		return;
	}
	
	// Do OnCompleted here!
	if(bIsCompleted){
		REBERU_LOG_ARGS(Log, "Reberu Level Placement complete! Created %d levels!", MovesList.Num())
		Output = EFinalizeRoomsOutputPins::OnCompleted;
		LevelGenerator->OnGenerationCompleted.Broadcast();
		Response.FinishAndTriggerIf(true, LatentActionInfo.ExecutionFunction, LatentActionInfo.Linkage, LatentActionInfo.CallbackTarget);
	}
}
