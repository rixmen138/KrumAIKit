#include "AddFunctionTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"

FString FAddFunctionTool::GetName() const
{
	return TEXT("AddFunction");
}

FString FAddFunctionTool::GetDescription() const
{
	return TEXT("Adds a new function to a Blueprint.");
}

TSharedPtr<FJsonObject> FAddFunctionTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShareable(new FJsonObject());
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("function_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("blueprint_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("function_name"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FAddFunctionTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath, FuncName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), AssetPath) ||
		!Params->TryGetStringField(TEXT("function_name"), FuncName))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return TEXT("{\"error\": \"Could not load Blueprint\"}");
	}

	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, *FuncName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	if (!NewGraph)
	{
		return TEXT("{\"error\": \"Failed to create new graph\"}");
	}

	FBlueprintEditorUtils::AddFunctionGraph<UClass>(Blueprint, NewGraph, false, nullptr);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

	return TEXT("{\"status\": \"success\"}");
}
