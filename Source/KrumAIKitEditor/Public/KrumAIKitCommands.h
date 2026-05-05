#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "KrumAIKitStyle.h"

class FKrumAIKitCommands : public TCommands<FKrumAIKitCommands>
{
public:

	FKrumAIKitCommands()
		: TCommands<FKrumAIKitCommands>(TEXT("KrumAIKit"), NSLOCTEXT("Contexts", "KrumAIKit", "KrumAIKit Plugin"), NAME_None, FKrumAIKitStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
