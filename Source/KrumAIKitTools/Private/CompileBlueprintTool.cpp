#include "CompileBlueprintTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"

FString FCompileBlueprintTool::GetName() const
{
	return TEXT("CompileBlueprint");
}

FString FCompileBlueprintTool::GetDescription() const
{
	return TEXT("Compiles a Blueprint asset.");
}

TSharedPtr<FJsonObject> FCompileBlueprintTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("blueprint_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("blueprint_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FCompileBlueprintTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), AssetPath))
	{
		return TEXT("{\"error\": \"Missing required parameter\"}");
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return TEXT("{\"error\": \"Could not load Blueprint\"}");
	}

	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	if (Blueprint->Status == BS_Error || Blueprint->Status == BS_Unknown)
	{
		return TEXT("{\"status\": \"error\", \"message\": \"Blueprint compilation failed with errors\"}");
	}

	return TEXT("{\"status\": \"success\"}");
}
