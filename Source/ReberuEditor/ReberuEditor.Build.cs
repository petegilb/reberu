using UnrealBuildTool;

public class ReberuEditor : ModuleRules
{
    public ReberuEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Reberu"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "ComponentVisualizers",
                "RenderCore",
                "ToolMenus",
                "UMG",
                "UMGEditor",
                "Blutility",
                "Reberu",
                "GameplayTags"
            }
        );
    }
}