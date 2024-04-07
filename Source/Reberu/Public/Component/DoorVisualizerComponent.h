// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DoorVisualizerComponent.generated.h"


class ARoomBounds;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REBERU_API UDoorVisualizerComponent : public UActorComponent{
	GENERATED_BODY()

public:
	UDoorVisualizerComponent();

	UFUNCTION(BlueprintPure)
	ARoomBounds* GetRoomBounds() const;

protected:
	virtual void BeginPlay() override;
	
};
