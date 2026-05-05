#include "ReadAssetTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "KrumAssetReader.h"

FString FReadAssetTool::GetName() const
{
	return TEXT("ReadAsset");
}

FString FReadAssetTool::GetDescription() const
{
	return TEXT("Returns a rich JSON description of any UE5 asset by its path.");
}

TSharedPtr<FJsonObject> FReadAssetTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("asset_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("asset_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FReadAssetTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return TEXT("{\"error\": \"Missing required parameter: asset_path\"}");
	}

	return FKrumAssetReader::ReadAsset(AssetPath);
}
