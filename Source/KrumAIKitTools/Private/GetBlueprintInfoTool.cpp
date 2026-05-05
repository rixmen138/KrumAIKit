#include "GetBlueprintInfoTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

FString FGetBlueprintInfoTool::GetName() const
{
	return TEXT("GetBlueprintInfo");
}

FString FGetBlueprintInfoTool::GetDescription() const
{
	return TEXT("Loads a Blueprint asset by path and returns its parent class, variables, functions, and components.");
}

TSharedPtr<FJsonObject> FGetBlueprintInfoTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("The package path of the Blueprint (e.g. /Game/Blueprints/BP_MyActor)."));
	Properties->SetObjectField(TEXT("asset_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("asset_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FGetBlueprintInfoTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return TEXT("{\"error\": \"Missing required parameter: asset_path\"}");
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return FString::Printf(TEXT("{\"error\": \"Could not load Blueprint at path '%s'\"}"), *AssetPath);
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	
	// Parent class
	if (Blueprint->ParentClass)
	{
		ResultObj->SetStringField(TEXT("parent_class"), Blueprint->ParentClass->GetName());
	}

	// Variables
	TArray<TSharedPtr<FJsonValue>> VarsArray;
	for (const FBPVariableDescription& VarDesc : Blueprint->NewVariables)
	{
		TSharedPtr<FJsonObject> VarObj = MakeShareable(new FJsonObject());
		VarObj->SetStringField(TEXT("name"), VarDesc.VarName.ToString());
		VarObj->SetStringField(TEXT("type"), VarDesc.VarType.PinCategory.ToString());
		VarObj->SetStringField(TEXT("sub_type"), VarDesc.VarType.PinSubCategoryObject ? VarDesc.VarType.PinSubCategoryObject->GetName() : TEXT(""));
		VarObj->SetStringField(TEXT("default_value"), VarDesc.DefaultValue);
		VarsArray.Add(MakeShareable(new FJsonValueObject(VarObj)));
	}
	ResultObj->SetArrayField(TEXT("variables"), VarsArray);

	// Components
	TArray<TSharedPtr<FJsonValue>> CompsArray;
	if (Blueprint->SimpleConstructionScript)
	{
		for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (Node && Node->ComponentClass)
			{
				TSharedPtr<FJsonObject> CompObj = MakeShareable(new FJsonObject());
				CompObj->SetStringField(TEXT("name"), Node->GetVariableName().ToString());
				CompObj->SetStringField(TEXT("class"), Node->ComponentClass->GetName());
				CompsArray.Add(MakeShareable(new FJsonValueObject(CompObj)));
			}
		}
	}
	ResultObj->SetArrayField(TEXT("components"), CompsArray);

	// Functions (Graphs)
	TArray<TSharedPtr<FJsonValue>> FuncsArray;
	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (Graph)
		{
			TSharedPtr<FJsonObject> FuncObj = MakeShareable(new FJsonObject());
			FuncObj->SetStringField(TEXT("name"), Graph->GetName());
			FuncsArray.Add(MakeShareable(new FJsonValueObject(FuncObj)));
		}
	}
	ResultObj->SetArrayField(TEXT("functions"), FuncsArray);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);

	return ResultString;
}
