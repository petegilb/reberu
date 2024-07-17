// Copyright Peter Gilbert, All Rights Reserved


#include "Library/ReberuLibrary.h"
#include "Reberu.h"
#include "Engine/LatentActionManager.h"
#include "Task/FinalizeRoomsTask.h"
#include "Task/GenerateRoomsTask.h"

// TODO can probably refactor this code as well as the tasks to be more reusable.

void UReberuLibrary::GenerateRooms(UObject* WorldContext, FLatentActionInfo LatentInfo, EGenerateRoomsInputPins InputPins, EGenerateRoomsOutputPins& OutputPins,
                                   ALevelGeneratorActor* LevelGenerator, UReberuData* ReberuData, const int32 Seed, const bool DebugDelay,
                                   const FTransform StartRoomTransform, int32& OutGeneratedRooms, bool&bOutSuccess)
{

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);

	if(World == nullptr){
		bOutSuccess = false;

		REBERU_LOG(Error, "World is invalid so we failed execution of the GenerateRooms BP node!")
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	FGenerateRoomsAction* ExistingAction = LatentActionManager.FindExistingAction<FGenerateRoomsAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

	if (InputPins == EGenerateRoomsInputPins::Start){
		if(!ExistingAction){
			FGenerateRoomsAction* NewAction = new FGenerateRoomsAction(
				LevelGenerator, ReberuData, OutGeneratedRooms, bOutSuccess, LatentInfo, OutputPins,
				Seed, DebugDelay, StartRoomTransform);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
	}
	else if(InputPins == EGenerateRoomsInputPins::Cancel){
		if(ExistingAction!= nullptr){
			ExistingAction->bWantToCancel = true;
		}
	}
}

void UReberuLibrary::FinalizeRooms(UObject* WorldContext, UReberuData* ReberuData, FLatentActionInfo LatentInfo, EFinalizeRoomsInputPins InputPins, EFinalizeRoomsOutputPins& OutputPins,
	ALevelGeneratorActor* LevelGenerator, bool& bOutSuccess){

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);

	if(World == nullptr){
		bOutSuccess = false;

		REBERU_LOG(Error, "World is invalid so we failed execution of the FinalizeRooms BP node!")
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	FFinalizeRoomsTask* ExistingAction = LatentActionManager.FindExistingAction<FFinalizeRoomsTask>(LatentInfo.CallbackTarget, LatentInfo.UUID);

	if (InputPins == EFinalizeRoomsInputPins::Start){
		if(!ExistingAction){
			// ALevelGeneratorActor* LevelGenerator, bool& bSuccess, const FLatentActionInfo& LatentActionInfo, EReberuTaskOutputPins& Output
			FFinalizeRoomsTask* NewAction = new FFinalizeRoomsTask(LevelGenerator, ReberuData, bOutSuccess, LatentInfo, OutputPins);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
	}
	else if(InputPins == EFinalizeRoomsInputPins::Cancel){
		if(ExistingAction!= nullptr){
			ExistingAction->bWantToCancel = true;
		}
	}
}

void UReberuLibrary::RegenerateDoorIds(UReberuRoomData* RoomData){
	for (FReberuDoor& Door : RoomData->Room.ReberuDoors){
		Door.GenerateNewDoorId();
	}
}
