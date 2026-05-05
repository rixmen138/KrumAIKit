#include "AddVariableTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "GameFramework/Actor.h"

FString FAddVariableTool::GetName() const
{
	return TEXT("AddVariable");
}

FString FAddVariableTool::GetDescription() const
{
	return TEXT("Adds a new variable to a Blueprint.");
}

TSharedPtr<FJsonObject> FAddVariableTool::GetSchema() const
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

	TSharedPtr<FJsonObject> TypeProp = MakeShareable(new FJsonObject());
	TypeProp->SetStringField(TEXT("type"), TEXT("string"));
	TypeProp->SetStringField(TEXT("description"), TEXT("Variable type: bool, int, float, string, vector, actor"));
	Properties->SetObjectField(TEXT("var_type"), TypeProp);

	TSharedPtr<FJsonObject> DefaultProp = MakeShareable(new FJsonObject());
	DefaultProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("default_value"), DefaultProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("blueprint_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("var_name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("var_type"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FAddVariableTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath, VarName, VarType, DefaultValue;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), AssetPath) ||
		!Params->TryGetStringField(TEXT("var_name"), VarName) ||
		!Params->TryGetStringField(TEXT("var_type"), VarType))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	Params->TryGetStringField(TEXT("default_value"), DefaultValue);

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return TEXT("{\"error\": \"Could not load Blueprint\"}");
	}

	FName CategoryName = UEdGraphSchema_K2::PC_Boolean;
	UObject* SubCategoryObject = nullptr;

	VarType = VarType.ToLower();
	if (VarType == TEXT("int")) CategoryName = UEdGraphSchema_K2::PC_Int;
	else if (VarType == TEXT("float")) CategoryName = UEdGraphSchema_K2::PC_Real;
	else if (VarType == TEXT("string")) CategoryName = UEdGraphSchema_K2::PC_String;
	else if (VarType == TEXT("vector")) 
	{
		CategoryName = UEdGraphSchema_K2::PC_Struct;
		SubCategoryObject = TBaseStructure<FVector>::Get();
	}
	else if (VarType == TEXT("actor"))
	{
		CategoryName = UEdGraphSchema_K2::PC_Object;
		SubCategoryObject = AActor::StaticClass();
	}

	FEdGraphPinType PinType(CategoryName, NAME_None, SubCategoryObject, EPinContainerType::None, false, FEdGraphTerminalType());
	
	FName NewVarName = FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, *VarName);
	
	if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, NewVarName, PinType, DefaultValue))
	{
		return TEXT("{\"error\": \"Failed to add variable\"}");
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

	return TEXT("{\"status\": \"success\"}");
}
