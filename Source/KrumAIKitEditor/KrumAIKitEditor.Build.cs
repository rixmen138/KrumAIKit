using UnrealBuildTool;

public class KrumAIKitEditor : ModuleRules
{
	public KrumAIKitEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"InputCore",
				"Json",
				"JsonUtilities",
				"Projects",
				"KrumAIKitAgents"
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
				"WorkspaceMenuStructure",
				"EditorStyle",
				"ToolMenus",
				"KrumAIKitCore",
				"KrumAIKitTools"
			}
		);
	}
}
