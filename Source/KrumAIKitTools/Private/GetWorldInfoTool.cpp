#include "GetWorldInfoTool.h"
#include "Engine/World.h"
#include "Editor.h"
#include "Serialization/JsonSerializer.h"

FString FGetWorldInfoTool::GetName() const
{
	return TEXT("GetWorldInfo");
}

FString FGetWorldInfoTool::GetDescription() const
{
	return TEXT("Returns basic information about the current editor world (level name, number of actors).");
}

TSharedPtr<FJsonObject> FGetWorldInfoTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	Schema->SetObjectField(TEXT("properties"), MakeShareable(new FJsonObject()));
	return Schema;
}

FString FGetWorldInfoTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		return TEXT("{\"error\": \"No active editor world\"}");
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("LevelName"), World->GetMapName());
	ResultObj->SetNumberField(TEXT("ActorCount"), World->GetActorCount());

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);

	return ResultString;
}
