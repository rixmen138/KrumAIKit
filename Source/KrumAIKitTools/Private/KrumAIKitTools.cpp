#include "KrumAIKitTools.h"
#include "KrumToolRegistry.h"
#include "CreateBlueprintTool.h"
#include "GetSelectedActorsTool.h"
#include "GetWorldInfoTool.h"
#include "GetBlueprintInfoTool.h"
#include "AddVariableTool.h"
#include "AddComponentTool.h"
#include "CompileBlueprintTool.h"
#include "AddFunctionTool.h"
#include "SetBlueprintVariableDefaultTool.h"

void FKrumAIKitToolsModule::StartupModule()
{
	auto& Registry = FKrumToolRegistry::Get();
	Registry.RegisterTool(MakeShared<FCreateBlueprintTool>());
	Registry.RegisterTool(MakeShared<FGetSelectedActorsTool>());
	Registry.RegisterTool(MakeShared<FGetWorldInfoTool>());
	Registry.RegisterTool(MakeShared<FGetBlueprintInfoTool>());
	Registry.RegisterTool(MakeShared<FAddVariableTool>());
	Registry.RegisterTool(MakeShared<FAddComponentTool>());
	Registry.RegisterTool(MakeShared<FCompileBlueprintTool>());
	Registry.RegisterTool(MakeShared<FAddFunctionTool>());
	Registry.RegisterTool(MakeShared<FSetBlueprintVariableDefaultTool>());
}

void FKrumAIKitToolsModule::ShutdownModule()
{
	FKrumToolRegistry::Get().UnregisterAll();
}

IMPLEMENT_MODULE(FKrumAIKitToolsModule, KrumAIKitTools)
