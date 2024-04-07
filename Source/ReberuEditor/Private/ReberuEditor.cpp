#include "ReberuEditor.h"

#include "UnrealEdGlobals.h"
#include "Component/DoorVisualizerComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "Visualizer/DoorComponentVisualizer.h"

DEFINE_LOG_CATEGORY(LogReberuEditor)

#define LOCTEXT_NAMESPACE "FReberuEditorModule"

void FReberuEditorModule::StartupModule()
{
	if (GUnrealEd)
	{
		const TSharedPtr<FDoorComponentVisualizer> Visualizer = MakeShareable(new FDoorComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UDoorVisualizerComponent::StaticClass()->GetFName(), Visualizer);
		Visualizer->OnRegister();
	}
}

void FReberuEditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UDoorVisualizerComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FReberuEditorModule, ReberuEditor)