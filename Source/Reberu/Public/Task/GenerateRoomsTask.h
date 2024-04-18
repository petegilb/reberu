// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LevelGeneratorActor.h"
#include "GenerateRoomsTask.generated.h"

UENUM()
enum class EGenerateRoomsInputPins : uint8{
	Start,
	Cancel
};

UENUM()
enum class EGenerateRoomsOutputPins : uint8{
	OnStarted,
	OnRoomPlaced,
	OnCancelled,
	OnFailed,
	OnCompleted
};

/** Latent action for generating rooms without blocking the thread. */
class REBERU_API FGenerateRoomsAction : public FPendingLatentAction{
public:

	// Local vars
	ALevelGeneratorActor* LevelGenerator = nullptr;
	UReberuData* ReberuData = nullptr;
	bool bIsFirstCall = true;
	bool bWantToCancel = false;
	bool bIsCompleted = false;

	TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode* SourceRoomNode = nullptr;
	int32 MaxBacktrackTries = 0;
	TSet<UReberuRoomData*> AttemptedNewRooms;

	// References
	TDoubleLinkedList<FReberuMove>& MovesList;
	FRandomStream& ReberuRandomStream;
	int32& GeneratedRooms;
	bool& bSuccess;
	

	FLatentActionInfo LatentActionInfo;
	EGenerateRoomsOutputPins& Output;

	FGenerateRoomsAction(ALevelGeneratorActor* LevelGenerator, UReberuData* ReberuData, int32& OutGeneratedRooms,
		bool& bOutSuccess, const FLatentActionInfo& LatentActionInfo, EGenerateRoomsOutputPins& OutputPins)
		: LevelGenerator(LevelGenerator),
		  ReberuData(ReberuData),
		  MovesList(LevelGenerator->GetMovesListRef()),
		  ReberuRandomStream(LevelGenerator->GetReberuRandomStream()),
		  GeneratedRooms(OutGeneratedRooms),
		  bSuccess(bOutSuccess),
		  LatentActionInfo(LatentActionInfo),
		  Output(OutputPins)
	{
		Output = EGenerateRoomsOutputPins::OnStarted;
		bSuccess = false;
		GeneratedRooms = 0;
		MaxBacktrackTries = ReberuData->MaxBacktrackTries;
	}

	virtual void UpdateOperation(FLatentResponse& Response) override;

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return FString::Printf(TEXT("Generating rooms using %s (%d so far!)"), *ReberuData->GetName(), GeneratedRooms);
	}
#endif
};