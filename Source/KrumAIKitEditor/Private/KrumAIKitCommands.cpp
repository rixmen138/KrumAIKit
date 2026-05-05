#include "KrumAIKitCommands.h"

#define LOCTEXT_NAMESPACE "FKrumAIKitModule"

void FKrumAIKitCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "KrumAI Chat", "Bring up KrumAI window", EUserInterfaceActionType::Button, FInputChord(EKeys::K, EModifierKey::Control | EModifierKey::Shift));
}

#undef LOCTEXT_NAMESPACE
