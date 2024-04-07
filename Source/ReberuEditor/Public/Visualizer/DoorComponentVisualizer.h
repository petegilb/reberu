// Copyright Peter Gilbert, All Rights Reserved

#pragma once

#include "ComponentVisualizer.h"

/**
 * Component visualizer to see the representation of a door in the scene.
 * I just realized how confusing it is to have DoorComponentVisualizer and DoorVisualizerComponent but oh well.
 */
class FDoorComponentVisualizer : public FComponentVisualizer
{
public:
	// Override this to draw in the scene
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	
	// Override this to draw on the editor's viewport
	// virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
	// 	const FSceneView* View, FCanvas* Canvas) override;
};