// Copyright Peter Gilbert, All Rights Reserved


#include "Component/DoorVisualizerComponent.h"

#include "RoomBounds.h"


UDoorVisualizerComponent::UDoorVisualizerComponent(){
	PrimaryComponentTick.bCanEverTick = false;
}

ARoomBounds* UDoorVisualizerComponent::GetRoomBounds() const{
	if(ARoomBounds* RoomBounds = Cast<ARoomBounds>(GetOwner())){
		return RoomBounds;
	}
	return nullptr;
}


void UDoorVisualizerComponent::BeginPlay(){
	Super::BeginPlay();
	
}

