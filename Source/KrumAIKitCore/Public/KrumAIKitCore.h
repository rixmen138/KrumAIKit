#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

KRUMAIKITCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogKrumAIKit, Log, All);

class FKrumAIKitCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
