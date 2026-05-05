using UnrealBuildTool;

public class KrumAIKitReaders : ModuleRules
{
	public KrumAIKitReaders(ReadOnlyTargetRules Target) : base(Target)
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
				"Json",
				"JsonUtilities",
				"KrumAIKitCore",
				"UnrealEd",
				"AssetRegistry",
				"AssetTools",
				"BlueprintGraph",
				"Kismet",
				"MaterialEditor",
				"AIModule",
				"GameplayTasks"
			}
		);
	}
}
