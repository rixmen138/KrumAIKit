#include "AddComponentTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectGlobals.h"

FString FAddComponentTool::GetName() const
{
	return TEXT("AddComponent");
}

FString FAddComponentTool::GetDescription() const
{
	return TEXT("Adds a new component to a Blueprint.");
}

TSharedPtr<FJsonObject> FAddComponentTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("blueprint_path"), PathProp);

	TSharedPtr<FJsonObject> ClassProp = MakeShareable(new FJsonObject());
	ClassProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("component_class"), ClassProp);

	TSharedPtr<FJsonObject> NameProp = MakeShareable(new FJsonObject());
	NameProp->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("component_name"), NameProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("blueprint_path"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("component_class"))));
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("component_name"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FAddComponentTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath, ComponentClassStr, ComponentName;
	if (!Params->TryGetStringField(TEXT("blueprint_path"), AssetPath) ||
		!Params->TryGetStringField(TEXT("component_class"), ComponentClassStr) ||
		!Params->TryGetStringField(TEXT("component_name"), ComponentName))
	{
		return TEXT("{\"error\": \"Missing required parameters\"}");
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	USimpleConstructionScript* SCS = Blueprint ? Blueprint->SimpleConstructionScript.Get() : nullptr;
	
	if (!Blueprint || !SCS)
	{
		return TEXT("{\"error\": \"Could not load Blueprint or it lacks a ConstructionScript\"}");
	}

	UClass* CompClass = FindFirstObject<UClass>(*ComponentClassStr, EFindFirstObjectOptions::None);
	if (!CompClass)
	{
		CompClass = FindFirstObject<UClass>(*FString::Printf(TEXT("/Script/Engine.%s"), *ComponentClassStr), EFindFirstObjectOptions::None);
	}

	if (!CompClass || !CompClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return TEXT("{\"error\": \"Invalid component class\"}");
	}

	USCS_Node* NewNode = SCS->CreateNode(CompClass, *ComponentName);
	if (!NewNode)
	{
		return TEXT("{\"error\": \"Failed to create node\"}");
	}

	SCS->AddNode(NewNode);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

	return TEXT("{\"status\": \"success\"}");
}
