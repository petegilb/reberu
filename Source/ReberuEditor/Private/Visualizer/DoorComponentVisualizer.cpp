// Copyright Peter Gilbert, All Rights Reserved

#include "Visualizer/DoorComponentVisualizer.h"

#include "EditorModes.h"
#include "RoomBounds.h"
#include "Component/DoorVisualizerComponent.h"
#include "SceneManagement.h"

void FDoorComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI){
	const UDoorVisualizerComponent* DoorComponent = Cast<UDoorVisualizerComponent>(Component);
	if (!DoorComponent) return;

	ARoomBounds* RoomBounds = DoorComponent->GetRoomBounds();
	const FTransform ComponentTransform = RoomBounds->GetRootComponent()->GetComponentTransform();
	const FVector DoorSpawnLocation = ComponentTransform.TransformPosition(RoomBounds->DoorSpawn);

	// If we don't have a selected door, display an arrow
	if(RoomBounds->CurrentlyEditingDoorId.IsEmpty()){
		const FMatrix Matrix = FScaleRotationTranslationMatrix(
			RoomBounds->GetRootComponent()->GetComponentScale(),
			FRotator(-90.f, 0.f, 0.f),
			DoorSpawnLocation + FVector(0.f, 0.f, 30.f));
		DrawDirectionalArrow(PDI, Matrix, FLinearColor::Blue, 30.f, 10.f, SDPG_World, 1.f);
		return;
	}

	// Otherwise display an AABB using the origin/extent method
	const FReberuDoor* CurrentDoor = RoomBounds->Doors.GetDoorById(RoomBounds->CurrentlyEditingDoorId);
	
	if(!CurrentDoor) return;
	
	const FMatrix Matrix = FScaleRotationTranslationMatrix(
			RoomBounds->GetRootComponent()->GetComponentScale(),
			RoomBounds->GetRootComponent()->GetComponentRotation(),
			DoorSpawnLocation);
	
	DrawWireBox(PDI, Matrix, FBox::BuildAABB(FVector(), CurrentDoor->BoxExtent), FLinearColor::Green, SDPG_World, 3.f);

	// auto* RenderProxy = new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), FLinearColor::Green);
	// PDI->RegisterDynamicResource(RenderProxy);
	// DrawBox(PDI, Matrix, BoxVector, RenderProxy, SDPG_World);
}
