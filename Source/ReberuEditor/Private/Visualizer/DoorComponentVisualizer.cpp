// Copyright Peter Gilbert, All Rights Reserved

#include "Visualizer/DoorComponentVisualizer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EditorModes.h"
#include "RoomBounds.h"
#include "Component/DoorVisualizerComponent.h"
#include "SceneManagement.h"

void FDoorComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI){
	const UDoorVisualizerComponent* DoorComponent = Cast<UDoorVisualizerComponent>(Component);
	if (!DoorComponent) return;
	
	ARoomBounds* RoomBounds = DoorComponent->GetRoomBounds();
	const FTransform RoomBoundsTransform = RoomBounds->GetActorTransform();
	const FTransform DoorSpawnWorldTransform = RoomBounds->DoorSpawn * RoomBoundsTransform;

	int32 CurrentDoorIdx = INDEX_NONE;
	
	// If we don't have a selected door, display an arrow
	if(RoomBounds->CurrentlyEditingDoorId.IsEmpty()){
		const FMatrix Matrix = DoorSpawnWorldTransform.ToMatrixWithScale();
		DrawDirectionalArrow(PDI, Matrix, FLinearColor::Blue, 30.f, 10.f, SDPG_World, 1.f);
	}
	else if (const FReberuDoor* CurrentDoor = RoomBounds->Room.GetDoorById(RoomBounds->CurrentlyEditingDoorId)){
		CurrentDoorIdx = RoomBounds->Room.GetDoorIdxById(RoomBounds->CurrentlyEditingDoorId);
		DrawDoor(View, PDI, DoorSpawnWorldTransform, CurrentDoor->BoxExtent, CurrentDoorIdx, FLinearColor::Yellow, FLinearColor::Red);
	}

	// Display all other doors
	TArray<FReberuDoor> DoorsToDraw = RoomBounds->Room.ReberuDoors;
	int32 DoorIdx = 0;
	for (FReberuDoor Door : DoorsToDraw){
		if(CurrentDoorIdx == INDEX_NONE || DoorIdx != CurrentDoorIdx){
			DrawDoor(View, PDI, Door.DoorTransform * RoomBoundsTransform, Door.BoxExtent, DoorIdx, FLinearColor::Green, FLinearColor::Red);
		}
		DoorIdx++;
	}
}

void FDoorComponentVisualizer::DrawDoor(const FSceneView* View, FPrimitiveDrawInterface* PDI, const FTransform& DoorWorldTransform, const FVector& DoorExtent,
	const int32 DoorIndex, const FLinearColor Color, FLinearColor TextColor)
{
	const FMatrix Matrix = DoorWorldTransform.ToMatrixWithScale();

	DrawDirectionalArrow(PDI, Matrix, FLinearColor::Blue, 30.f, 10.f, SDPG_World, 1.f);

	// display an AABB using the origin/extent method
	DrawWireBox(PDI, Matrix, FBox::BuildAABB(FVector(), DoorExtent), Color, SDPG_World, 3.f);

	FString WorldString = "Door Idx: ";
	WorldString += FString::FromInt(DoorIndex);
	DrawWorldText(DoorWorldTransform.GetLocation() + FVector(0.f, 0.f, 20.f), View, WorldString, TextColor);
}

void FDoorComponentVisualizer::DrawWorldText(const FVector& InWorldLocation, const FSceneView* InView, const FString& InString, FLinearColor Color){
	// https://forums.unrealengine.com/t/how-to-draw-world-space-text-for-a-custom-editor-mode/480200/3
	FVector TextPosition = InView->Project(InWorldLocation);
	if(TextPosition.X < 1.0f && TextPosition.X > -1.0f && (TextPosition.Y < 1.0f || TextPosition.Y > -1.0f)){
		FViewport* Viewport = GEditor->GetActiveViewport();
		FCanvas* DebugCanvas = Viewport->GetDebugCanvas();

		const FIntPoint ViewportSize = Viewport->GetSizeXY();
		TextPosition = (TextPosition + FVector::OneVector) / 2.0f;
		TextPosition.Y = 1.0f - TextPosition.Y;
		TextPosition *= FVector(ViewportSize);
		
		FCanvasTextItem TextItem(FVector2d(TextPosition.X, TextPosition.Y), FText::GetEmpty(), GEngine->GetMediumFont(), Color);
		TextItem.Scale = FVector2d(1,1);
		TextItem.DisableShadow();
		TextItem.Text = FText::FromString(InString);
		DebugCanvas->DrawItem(TextItem);
	}
}
