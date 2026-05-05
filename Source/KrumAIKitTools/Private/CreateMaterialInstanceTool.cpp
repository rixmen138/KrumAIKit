#include "CreateMaterialInstanceTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"

FString FCreateMaterialInstanceTool::GetName() const
{
	return TEXT("CreateMaterialInstance");
}

FString FCreateMaterialInstanceTool::GetDescription() const
{
	return TEXT("Creates a new Material Instance from a parent material.");
}

TSharedPtr<FJsonObject> FCreateMaterialInstanceTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> ParentProp = MakeShareable(new FJsonObject());
	ParentProp->SetStringField(TEXT("type"), TEXT("string"));
	ParentProp->SetStringField(TEXT("description"), TEXT("Full path of the parent material."));
	Properties->SetObjectField(TEXT("parent_path"), ParentProp);

	TSharedPtr<FJsonObject> InstProp = MakeShareable(new FJsonObject());
	InstProp->SetStringField(TEXT("type"), TEXT("string"));
	InstProp->SetStringField(TEXT("description"), TEXT("Full path for the new instance."));
	Properties->SetObjectField(TEXT("instance_path"), InstProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("parent_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("instance_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FCreateMaterialInstanceTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString ParentPath, InstancePath;
	if (!Params->TryGetStringField(TEXT("parent_path"), ParentPath) ||
		!Params->TryGetStringField(TEXT("instance_path"), InstancePath))
	{
		return TEXT("{\"error\": \"Missing required parameters: parent_path, instance_path\"}");
	}

	UMaterial* Parent = LoadObject<UMaterial>(nullptr, *ParentPath);
	if (!Parent)
	{
		return TEXT("{\"error\": \"Failed to load parent material.\"}");
	}

	FString InstancePackagePath = FPaths::GetPath(InstancePath);
	FString InstanceName = FPaths::GetBaseFilename(InstancePath);

	UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(
		FAssetToolsModule::GetModule().Get().CreateAsset(
			InstanceName, InstancePackagePath, UMaterialInstanceConstant::StaticClass(), nullptr));

	if (!MIC)
	{
		return TEXT("{\"error\": \"Failed to create Material Instance asset.\"}");
	}

	MIC->Parent = Parent;
	FAssetRegistryModule::AssetCreated(MIC);
	MIC->MarkPackageDirty();

	return TEXT("{\"status\": \"success\"}");
}
