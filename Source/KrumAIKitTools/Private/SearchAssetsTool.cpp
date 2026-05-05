#include "SearchAssetsTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "KrumAssetReader.h"

FString FSearchAssetsTool::GetName() const
{
	return TEXT("SearchAssets");
}

FString FSearchAssetsTool::GetDescription() const
{
	return TEXT("Searches the Content Browser for assets matching a query.");
}

TSharedPtr<FJsonObject> FSearchAssetsTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> QueryProp = MakeShareable(new FJsonObject());
	QueryProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("query"), QueryProp);

	TSharedPtr<FJsonObject> ClassProp = MakeShareable(new FJsonObject());
	ClassProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("class_filter"), ClassProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("query"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FSearchAssetsTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString Query, ClassFilter;
	if (!Params->TryGetStringField(TEXT("query"), Query))
	{
		return TEXT("{\"error\": \"Missing required parameter: query\"}");
	}

	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);

	return FKrumAssetReader::SearchAssets(Query, ClassFilter);
}
