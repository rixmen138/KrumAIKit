#include "CreateBehaviorTreeTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"

FString FCreateBehaviorTreeTool::GetName() const
{
	return TEXT("CreateBehaviorTree");
}

FString FCreateBehaviorTreeTool::GetDescription() const
{
	return TEXT("Creates a new Behavior Tree and an associated Blackboard.");
}

TSharedPtr<FJsonObject> FCreateBehaviorTreeTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Full path of the new BT (e.g. /Game/AI/BT_Guard)."));
	Properties->SetObjectField(TEXT("asset_path"), PathProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("asset_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FCreateBehaviorTreeTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return TEXT("{\"error\": \"Missing required parameter: asset_path\"}");
	}

	FString PackagePath = FPaths::GetPath(AssetPath);
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	UBehaviorTree* BT = Cast<UBehaviorTree>(FAssetToolsModule::GetModule().Get().CreateAsset(AssetName, PackagePath, UBehaviorTree::StaticClass(), nullptr));
	if (!BT)
	{
		return TEXT("{\"error\": \"Failed to create BehaviorTree asset.\"}");
	}

	FString BBName = AssetName + TEXT("_BB");
	UBlackboardData* BB = Cast<UBlackboardData>(FAssetToolsModule::GetModule().Get().CreateAsset(BBName, PackagePath, UBlackboardData::StaticClass(), nullptr));
	
	if (BB)
	{
		BT->BlackboardAsset = BB;
		FAssetRegistryModule::AssetCreated(BB);
		BB->MarkPackageDirty();
	}

	FAssetRegistryModule::AssetCreated(BT);
	BT->MarkPackageDirty();

	FString ResultString = FString::Printf(TEXT("{\"status\": \"success\", \"bt_path\": \"%s\", \"bb_path\": \"%s/%s\"}"), *AssetPath, *PackagePath, *BBName);
	return ResultString;
}
