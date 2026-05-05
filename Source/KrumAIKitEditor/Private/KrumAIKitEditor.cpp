#include "KrumAIKitEditor.h"
#include "KrumAIKitStyle.h"
#include "KrumAIKitCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "SKrumChatWindow.h"

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
}

void FKrumAIKitEditorModule::ShutdownModule()
{
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
