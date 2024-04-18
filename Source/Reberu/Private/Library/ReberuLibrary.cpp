// Copyright Peter Gilbert, All Rights Reserved


#include "Library/ReberuLibrary.h"
#include "Reberu.h"
#include "Engine/LatentActionManager.h"
#include "Task/GenerateRoomsTask.h"


void UReberuLibrary::GenerateRooms(UObject* WorldContext, FLatentActionInfo LatentInfo, EGenerateRoomsInputPins InputPins, EGenerateRoomsOutputPins& OutputPins,
                                   ALevelGeneratorActor* LevelGenerator, UReberuData* ReberuData, int32& OutGeneratedRooms, bool& bOutSuccess){

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
			FGenerateRoomsAction* NewAction = new FGenerateRoomsAction(LevelGenerator, ReberuData, OutGeneratedRooms, bOutSuccess, LatentInfo, OutputPins);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
	}
	else if(InputPins == EGenerateRoomsInputPins::Cancel){
		if(ExistingAction!= nullptr){
			ExistingAction->bWantToCancel = true;
		}
	}
}
