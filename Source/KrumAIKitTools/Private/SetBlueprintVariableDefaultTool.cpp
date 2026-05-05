#include "SetBlueprintVariableDefaultTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

FString FSetBlueprintVariableDefaultTool::GetName() const
{
	return TEXT("SetBlueprintVariableDefault");
}

FString FSetBlueprintVariableDefaultTool::GetDescription() const
{
	return TEXT("Sets the default value of an existing variable in a Blueprint.");
}

TSharedPtr<FJsonObject> FSetBlueprintVariableDefaultTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> NameProp = MakeShareable(new FJsonObject());
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("var_name"), NameProp);

	TSharedPtr<FJsonObject> DefProp = MakeShareable(new FJsonObject());
	DefProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("default_value"), DefProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("blueprint_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("var_name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("default_value"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FSetBlueprintVariableDefaultTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath, VarName, DefaultValue;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), AssetPath) ||
		!Params->TryGetStringField(TEXT("var_name"), VarName) ||
		!Params->TryGetStringField(TEXT("default_value"), DefaultValue))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return TEXT("{\"error\": \"Could not load Blueprint\"}");
	}

	int32 VarIndex = Blueprint->NewVariables.IndexOfByPredicate([&](const FBPVariableDescription& Var) {
		return Var.VarName.ToString() == VarName;
	});

	if (VarIndex == INDEX_NONE)
	{
		return TEXT("{\"error\": \"Variable not found in Blueprint\"}");
	}

	// TODO: UE 5.7 removed FBlueprintEditorUtils::SetBlueprintVariableDefaultValue.
	// We need to implement default value setting via UK2Node or Metadata directly in the future.
	// For now, return an error.
	return TEXT("{\"error\": \"Not implemented: SetBlueprintVariableDefaultValue is deprecated in UE5.7.\"}");
}
