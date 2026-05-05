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
#include "CreateMaterialTool.h"
#include "CreateMaterialInstanceTool.h"
#include "SetMaterialScalarParamTool.h"
#include "SetMaterialVectorParamTool.h"
#include "CreateBehaviorTreeTool.h"
#include "AddBlackboardKeyTool.h"
#include "ReadAssetTool.h"
#include "SearchAssetsTool.h"

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
	Registry.RegisterTool(MakeShared<FCreateMaterialTool>());
	Registry.RegisterTool(MakeShared<FCreateMaterialInstanceTool>());
	Registry.RegisterTool(MakeShared<FSetMaterialScalarParamTool>());
	Registry.RegisterTool(MakeShared<FSetMaterialVectorParamTool>());
	Registry.RegisterTool(MakeShared<FCreateBehaviorTreeTool>());
	Registry.RegisterTool(MakeShared<FAddBlackboardKeyTool>());
	Registry.RegisterTool(MakeShared<FReadAssetTool>());
	Registry.RegisterTool(MakeShared<FSearchAssetsTool>());
}

void FKrumAIKitToolsModule::ShutdownModule()
{
	FKrumToolRegistry::Get().UnregisterAll();
}

IMPLEMENT_MODULE(FKrumAIKitToolsModule, KrumAIKitTools)
