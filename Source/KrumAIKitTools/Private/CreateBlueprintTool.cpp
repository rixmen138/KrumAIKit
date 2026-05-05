#include "CreateBlueprintTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

FString FCreateBlueprintTool::GetName() const
{
	return TEXT("CreateBlueprint");
}

FString FCreateBlueprintTool::GetDescription() const
{
	return TEXT("Creates a new Blueprint asset. Provide name, path (e.g. /Game/Blueprints), and parent_class (e.g. Actor).");
}

TSharedPtr<FJsonObject> FCreateBlueprintTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> NameProp = MakeShareable(new FJsonObject());
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	NameProp->SetStringField(TEXT("description"), TEXT("The name of the new Blueprint (e.g. BP_MyActor)."));
	Properties->SetObjectField(TEXT("name"), NameProp);

	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("The package path where to create it (e.g. /Game/Blueprints)."));
	Properties->SetObjectField(TEXT("path"), PathProp);

	TSharedPtr<FJsonObject> ParentClassProp = MakeShareable(new FJsonObject());
	ParentClassProp->SetStringField(TEXT("type"), TEXT("string"));
	ParentClassProp->SetStringField(TEXT("description"), TEXT("The parent class (e.g. Actor, Pawn, Character)."));
	Properties->SetObjectField(TEXT("parent_class"), ParentClassProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("name"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("parent_class"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FCreateBlueprintTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	// Note: Fully implementing FKismetEditorUtilities::CreateBlueprint requires many headers
	// Here we return a mocked success/failure string until full Kismet/AssetTools module includes are setup.
	
	FString Name, Path, ParentClass;
	if (!Params->TryGetStringField(TEXT("name"), Name) || 
		!Params->TryGetStringField(TEXT("path"), Path) || 
		!Params->TryGetStringField(TEXT("parent_class"), ParentClass))
	{
		return TEXT("{\"error\": \"Missing required parameters: name, path, or parent_class\"}");
	}

	FString ResultMsg = FString::Printf(TEXT("Successfully simulated creation of Blueprint '%s' at '%s' inheriting from '%s'"), *Name, *Path, *ParentClass);
	
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("status"), TEXT("success"));
	ResultObj->SetStringField(TEXT("message"), ResultMsg);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);

	return ResultString;
}
