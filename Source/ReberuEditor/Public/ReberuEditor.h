#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogReberuEditor, All, All)

class FReberuEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Extend the menu in the editor. Referenced: https://minifloppy.it/posts/2024/adding-custom-buttons-unreal-editor-toolbars-menus/#menus-in-the-unreal-editor  */
    void RegisterMenuExtensions();
};
