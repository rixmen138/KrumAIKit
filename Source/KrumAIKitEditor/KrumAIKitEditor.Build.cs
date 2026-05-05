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
				"KrumAIKitAgents",
				"KrumAIKitTools"
			}
		);
	}
}
