// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Task/GenerateRoomsTask.h"
#include "ReberuLibrary.generated.h"

/**
 * Blueprint function library for Reberu
 */
UCLASS()
class REBERU_API UReberuLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Generate Rooms using the LevelGenerator Actor
	 * 
	 * @param WorldContext The world context object
	 * @param LatentInfo Holds information regarding the latent info of this function
	 * @param InputPins Input pins on the BP node
	 * @param OutputPins Output pins on the BP node
	 * @param LevelGenerator Reference to the level generator that should exist in the world
	 * @param ReberuData The input ReberuData data asset that contains the rooms we will generate
	 * @param Seed The seed that reberu will use for generation. Less than 1 will be a random seed.
	 * @param DebugDelay If debug delay is enabled, the generation will proceed slower than usual so it can be visualized easier.
	 * @param OutGeneratedRooms The number of rooms generated
	 * @param bOutSuccess If the generation was a success 
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "InputPins,OutputPins"), Category="Reberu")
	static void GenerateRooms(UObject* WorldContext, FLatentActionInfo LatentInfo, EGenerateRoomsInputPins InputPins, EGenerateRoomsOutputPins& OutputPins,
	                          ALevelGeneratorActor* LevelGenerator, UReberuData* ReberuData, const int32 Seed, const bool DebugDelay, int32& OutGeneratedRooms, bool& bOutSuccess);
	
};
