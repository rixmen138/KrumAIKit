#include "KrumAIKitEditor.h"
#include "KrumAIKitStyle.h"
#include "KrumAIKitCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "SKrumChatWindow.h"
#include "KrumMCPServer.h"
#include "Misc/FileHelper.h"

static const FName KrumAITabName("KrumAI");

#define LOCTEXT_NAMESPACE "FKrumAIKitEditorModule"

void FKrumAIKitEditorModule::StartupModule()
{
	FKrumAIKitStyle::Initialize();
	FKrumAIKitStyle::ReloadTextures();

	FKrumAIKitCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FKrumAIKitCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FKrumAIKitEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FKrumAIKitEditorModule::RegisterMenus));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(KrumAITabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& SpawnTabArgs)
    {
        return SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            [
                SNew(SKrumChatWindow)
            ];
    }))
    .SetDisplayName(LOCTEXT("FKrumAITabTitle", "KrumAI Chat"))
    .SetMenuType(ETabSpawnerMenuType::Hidden);

	FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString MCPJsonPath = ProjectDir / TEXT(".mcp.json");

	if (!FPaths::FileExists(MCPJsonPath))
	{
		FString EditorBinaryPath = FPlatformProcess::ExecutablePath();
		FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
		
		FString MCPContent = FString::Printf(
			TEXT("{\n  \"mcpServers\": {\n    \"krumaikit\": {\n")
			TEXT("      \"command\": \"%s\",\n")
			TEXT("      \"args\": [\"%s\"],\n")
			TEXT("      \"env\": { \"KRUMAIKIT_MCP\": \"1\" }\n")
			TEXT("    }\n  }\n}"),
			*EditorBinaryPath.Replace(TEXT("\\"), TEXT("\\\\")),
			*ProjectPath.Replace(TEXT("\\"), TEXT("\\\\"))
		);
		FFileHelper::SaveStringToFile(MCPContent, *MCPJsonPath);
		UE_LOG(LogKrumAIKit, Log, TEXT("KrumAIKit: wrote .mcp.json to %s"), *MCPJsonPath);
	}

	// We will read settings to start MCPServer in a future step, for now just start it:
	// MCPServer = MakeUnique<FKrumMCPServer>(); MCPServer->Start();
}

void FKrumAIKitEditorModule::ShutdownModule()
{
	if (MCPServer)
	{
		MCPServer->Stop();
		MCPServer.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FKrumAIKitStyle::Shutdown();
	FKrumAIKitCommands::Unregister();
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(KrumAITabName);
}

void FKrumAIKitEditorModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(KrumAITabName);
}

void FKrumAIKitEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("KrumAI");
			Section.AddMenuEntryWithCommandList(FKrumAIKitCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FKrumAIKitEditorModule, KrumAIKitEditor)
