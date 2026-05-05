#include "GetSelectedActorsTool.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "Serialization/JsonSerializer.h"

FString FGetSelectedActorsTool::GetName() const
{
	return TEXT("GetSelectedActors");
}

FString FGetSelectedActorsTool::GetDescription() const
{
	return TEXT("Returns a JSON array containing information about the currently selected actors in the editor world.");
}

TSharedPtr<FJsonObject> FGetSelectedActorsTool::GetSchema() const
{
	// Empty parameters schema
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	Schema->SetObjectField(TEXT("properties"), MakeShareable(new FJsonObject()));
	return Schema;
}

FString FGetSelectedActorsTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor)
	{
		return TEXT("{\"error\": \"Editor is not available\"}");
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	TArray<TSharedPtr<FJsonValue>> ActorsJsonArray;

	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* Actor = Cast<AActor>(*Iter))
		{
			TSharedPtr<FJsonObject> ActorJson = MakeShareable(new FJsonObject());
			ActorJson->SetStringField(TEXT("Name"), Actor->GetActorLabel());
			ActorJson->SetStringField(TEXT("Class"), Actor->GetClass()->GetName());
			
			FVector Loc = Actor->GetActorLocation();
			ActorJson->SetStringField(TEXT("Location"), FString::Printf(TEXT("X=%.2f Y=%.2f Z=%.2f"), Loc.X, Loc.Y, Loc.Z));
			
			ActorsJsonArray.Add(MakeShareable(new FJsonValueObject(ActorJson)));
		}
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetArrayField(TEXT("SelectedActors"), ActorsJsonArray);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);

	return ResultString;
}
