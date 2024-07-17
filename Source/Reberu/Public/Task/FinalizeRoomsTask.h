// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LevelGeneratorActor.h"
#include "LatentActions.h"
#include "FinalizeRoomsTask.generated.h"

UENUM()
enum class EFinalizeRoomsInputPins : uint8{
	Start,
	Cancel
};

UENUM()
enum class EFinalizeRoomsOutputPins : uint8{
	OnStarted,
	OnLevelCreated,
	OnCancelled,
	OnFailed,
	OnCompleted
};

/** Latent action for transforming all room bounds in the move list into Level Instances. */
class REBERU_API FFinalizeRoomsTask : public FPendingLatentAction{
public:
	// Local Vars
	ALevelGeneratorActor* LevelGenerator = nullptr;
	bool bIsFirstCall = true;
	bool bWantToCancel = false; 
	bool bIsCompleted = false;
	int32 CurrentIdx = 0;
	UReberuData* ReberuData = nullptr;

	TDoubleLinkedList<FReberuMove>::TDoubleLinkedListNode* CurrentMove = nullptr;

	// References
	TDoubleLinkedList<FReberuMove>& MovesList;
	bool& bSuccess;

	FLatentActionInfo LatentActionInfo;
	EFinalizeRoomsOutputPins& Output;
	
	// Constructor
	FFinalizeRoomsTask(ALevelGeneratorActor* LevelGenerator, UReberuData* ReberuData, bool& bSuccess, const FLatentActionInfo& LatentActionInfo, EFinalizeRoomsOutputPins& Output)
		: LevelGenerator(LevelGenerator),
		  ReberuData(ReberuData),
		  MovesList(LevelGenerator->GetMovesListRef()),
		  bSuccess(bSuccess),
		  LatentActionInfo(LatentActionInfo),
		  Output(Output)
	{
		Output = EFinalizeRoomsOutputPins::OnStarted;
		bSuccess = false;
	}

	virtual void UpdateOperation(FLatentResponse& Response) override;

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return FString::Printf(TEXT("Finalizing rooms %d / %d"), CurrentIdx, MovesList.Num());
	}
#endif
};
