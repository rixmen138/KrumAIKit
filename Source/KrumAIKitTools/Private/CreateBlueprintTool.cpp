#include "CreateBlueprintTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/Blueprint.h"
#include "UObject/UObjectGlobals.h"

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
	FString Name, Path, ParentClassName;
	if (!Params->TryGetStringField(TEXT("name"), Name) || 
		!Params->TryGetStringField(TEXT("path"), Path) || 
		!Params->TryGetStringField(TEXT("parent_class"), ParentClassName))
	{
		return TEXT("{\"error\": \"Missing required parameters: name, path, or parent_class\"}");
	}

	UClass* ParentClass = FindFirstObject<UClass>(*ParentClassName, EFindFirstObjectOptions::None);
	if (!ParentClass)
	{
		// Try adding standard engine prefix if it fails (e.g., "Actor" -> "/Script/Engine.Actor")
		ParentClass = FindFirstObject<UClass>(*FString::Printf(TEXT("/Script/Engine.%s"), *ParentClassName), EFindFirstObjectOptions::None);
	}

	if (!ParentClass)
	{
		return FString::Printf(TEXT("{\"error\": \"Could not find parent class '%s'\"}"), *ParentClassName);
	}

	// Make sure path is valid and ends without a slash
	if (Path.EndsWith(TEXT("/")))
	{
		Path.LeftChopInline(1);
	}

	FString PackageName = FString::Printf(TEXT("%s/%s"), *Path, *Name);
	
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return FString::Printf(TEXT("{\"error\": \"Failed to create package at %s\"}"), *PackageName);
	}

	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		*Name,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName("KrumAIKitCreateBlueprint")
	);

	if (!NewBP)
	{
		return TEXT("{\"error\": \"FKismetEditorUtilities::CreateBlueprint failed\"}");
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetCreated(NewBP);
	Package->MarkPackageDirty();

	FString ResultMsg = FString::Printf(TEXT("Successfully created Blueprint '%s' inheriting from '%s'"), *PackageName, *ParentClass->GetName());
	
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetStringField(TEXT("status"), TEXT("success"));
	ResultObj->SetStringField(TEXT("message"), ResultMsg);

	FString ResultString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
	FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);

	return ResultString;
}
