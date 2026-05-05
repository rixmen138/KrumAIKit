#include "CreateMaterialTool.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Materials/Material.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"

FString FCreateMaterialTool::GetName() const
{
	return TEXT("CreateMaterial");
}

FString FCreateMaterialTool::GetDescription() const
{
	return TEXT("Creates a new Material asset at the given path.");
}

TSharedPtr<FJsonObject> FCreateMaterialTool::GetSchema() const
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
	PathProp->SetStringField(TEXT("type"), TEXT("string"));
	PathProp->SetStringField(TEXT("description"), TEXT("Full path of the new Material (e.g. /Game/Materials/M_Rock)."));
	Properties->SetObjectField(TEXT("asset_path"), PathProp);

	TSharedPtr<FJsonObject> ShadingProp = MakeShareable(new FJsonObject());
	ShadingProp->SetStringField(TEXT("type"), TEXT("string"));
	ShadingProp->SetStringField(TEXT("description"), TEXT("Shading model: DefaultLit, Unlit, Subsurface, etc."));
	Properties->SetObjectField(TEXT("shading_model"), ShadingProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	TArray<TSharedPtr<FJsonValue>> RequiredArray;
	RequiredArray.Add(MakeShareable(new FJsonValueString(TEXT("asset_path"))));
	Schema->SetArrayField(TEXT("required"), RequiredArray);

	return Schema;
}

FString FCreateMaterialTool::Execute(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath, ShadingModelStr;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return TEXT("{\"error\": \"Missing required parameter: asset_path\"}");
	}

	FString PackagePath = FPaths::GetPath(AssetPath);
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	UMaterial* Mat = Cast<UMaterial>(FAssetToolsModule::GetModule().Get().CreateAsset(AssetName, PackagePath, UMaterial::StaticClass(), nullptr));
	if (!Mat)
	{
		return TEXT("{\"error\": \"Failed to create Material asset.\"}");
	}

	if (Params->TryGetStringField(TEXT("shading_model"), ShadingModelStr))
	{
		EMaterialShadingModel ShadingModel = MSM_DefaultLit;
		if (ShadingModelStr == TEXT("Unlit")) ShadingModel = MSM_Unlit;
		else if (ShadingModelStr == TEXT("Subsurface")) ShadingModel = MSM_Subsurface;
		else if (ShadingModelStr == TEXT("PreintegratedSkin")) ShadingModel = MSM_PreintegratedSkin;
		else if (ShadingModelStr == TEXT("ClearCoat")) ShadingModel = MSM_ClearCoat;
		else if (ShadingModelStr == TEXT("SubsurfaceProfile")) ShadingModel = MSM_SubsurfaceProfile;
		else if (ShadingModelStr == TEXT("TwoSidedFoliage")) ShadingModel = MSM_TwoSidedFoliage;
		else if (ShadingModelStr == TEXT("Hair")) ShadingModel = MSM_Hair;
		else if (ShadingModelStr == TEXT("Cloth")) ShadingModel = MSM_Cloth;
		else if (ShadingModelStr == TEXT("Eye")) ShadingModel = MSM_Eye;
		else if (ShadingModelStr == TEXT("SingleLayerWater")) ShadingModel = MSM_SingleLayerWater;
		else if (ShadingModelStr == TEXT("ThinTranslucent")) ShadingModel = MSM_ThinTranslucent;
		
		Mat->SetShadingModel(ShadingModel);
	}

	FAssetRegistryModule::AssetCreated(Mat);
	Mat->MarkPackageDirty();

	FString ResultString = FString::Printf(TEXT("{\"status\": \"success\", \"asset_path\": \"%s\"}"), *AssetPath);
	return ResultString;
}
