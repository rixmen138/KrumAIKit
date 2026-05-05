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
				"KrumAIKitCore",
				"AssetTools"
			}
		);
	}
}
