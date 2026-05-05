using UnrealBuildTool;

public class KrumAIKitTools : ModuleRules
{
	public KrumAIKitTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"KrumAIKitCore"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"Json",
				"JsonUtilities",
				"AssetTools",
				"BlueprintGraph",
				"KismetCompiler",
				"GraphEditor",
				"Kismet",
				"Kismet2"
			}
		);
	}
}
