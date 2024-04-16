#include "ReberuEditor.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
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

	// Register a function to be called when menu system is initialized
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FReberuEditorModule::RegisterMenuExtensions));
}

void FReberuEditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UDoorVisualizerComponent::StaticClass()->GetFName());
	}

	// Unregister the startup function
	UToolMenus::UnRegisterStartupCallback(this);
 
	// Unregister all our menu extensions
	UToolMenus::UnregisterOwner(this);
}

void FReberuEditorModule::RegisterMenuExtensions(){
	// Referenced: https://minifloppy.it/posts/2024/adding-custom-buttons-unreal-editor-toolbars-menus
	// Use the current object as the owner of the menus
	// This allows us to remove all our custom menus when the 
	// module is unloaded (see ShutdownModule below)
	FToolMenuOwnerScoped OwnerScoped(this);
 
	// Extend the "File" section of the main toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
		"LevelEditor.LevelEditorToolBar.ModesToolBar");
	FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection("Content");
 
	ToolbarSection.AddEntry(FToolMenuEntry::InitToolBarButton(
		TEXT("ReberuButton"),
		FExecuteAction::CreateLambda([]()
		{
			// Simply log for this example
			UE_LOG(LogTemp, Log, TEXT("Reberu editor button triggered!!"));
			UEditorUtilityWidgetBlueprint* UMGBP = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, L"/Script/Blutility.EditorUtilityWidgetBlueprint'/Reberu/EUW_CreateRoomData.EUW_CreateRoomData'");
			if(!(UMGBP && GEditor)) return;
			UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
			if(EditorUtilitySubsystem) EditorUtilitySubsystem->SpawnAndRegisterTab(UMGBP);
		}),
		INVTEXT("Reberu button"),
		INVTEXT("Launches Reberu Editor Utility Widget"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "AssetEditor.ToggleShowBounds")
	));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FReberuEditorModule, ReberuEditor)